c***********************************************************************
c  Program to process raw AMS data and produce two files: one with the
c  principal directions and a second with the bootstrap error ellipse
c  data in various coordinate systems.
c
c  Compilation notes:
c    RAND function requires unix library (Libraries:Absoft)
c    use "fold to upper case"
c
c  date??	- added option for no bootstrap
c  5/04/01	- output of tensor (user specified coordinate system)
c		- option for no azdip file
c       	- added sigma to output of tensor file (used in findH.f)
c  3/30/02	- v2.0 
c 		- replaced rand function with ran2 from Num. Recipes
c 		- still requires "fold to upper case"
c  9/13/04      - Given to Kurt Schwehr by Jeff Gee.
c***********************************************************************

c Kent distribution stuff by Cathy.
c Comments and docs by Peter Selkin and Jeff Gee

c This version is setup for Mac OS 9 with the Absoft fortran compiler

	parameter(maxaz=3000)
	parameter(maxraw=3000)
	parameter(nb=1000)
	implicit double precision (a-h,o-z)
	
	character*30 outfile
	character*80 line
	character*10 azID(maxaz)	! array with IDs from azdip file
	dimension azdata(maxaz,4)	! array with az,pl,str,dip
	dimension cazdata(4)		! array with constant azdip info
	
	character*10 rawID(maxraw)	! array with IDs assoc with 15 meas
	dimension raw(maxraw,15)	! array with assoc 15 meas
	
	dimension x(15),a(3,3)		! for use in dok15_s
	dimension s(6),f(3),tau(3)	! for use in dohext
	dimension yell(3,8)

	dimension tens(maxraw,8)	! k11,k22,k33,k12,k23,k13,sigma,bulk
	
	character*10 siteID(maxaz)	! ordered list of unique site IDs
	integer width			! search width for site IDs
	character*1 ans
	character*10 site			! current sample or site ID
	character*10 blockID(100)	! IDs for data block from one site
	dimension data(100,8)		! associated sample tensor data
	
	dimension cdecl(3),cincl(3)	! data for output in lamont format
	dimension gdecl(3),gincl(3)	! note that order of data is
	dimension bdecl(3),bincl(3)	! straightened out here
	dimension eigen(3)		! 1 max, 2 int, 3 min
	
	dimension core(3,3)		! tensor in core coordinates
	dimension geo(3,3)		! tensor in geographic coords
	dimension tilt(3,3)		! tensor in tilt coords
		
	dimension sig(nb+1)		! block data for bs
	dimension sigsave(nb+1)		! block data for bs
	dimension sbs(6,nb)		! block of s in samp coord
	dimension gbs(6,nb)		! block of s in geog coord
	dimension tbs(6,nb)		! block of s in tilt coord
	
	dimension bstrap(3,10)		! output from bootams
	
	pi=2.0*dasin(1.0d0)
	rad=pi/180
	ipar=2				! initialize for -P option

c===================================================
c    Read in the azdip data and raw measurements
c===================================================

	write(*,*) '================================================='
	write(*,*) ' Program to merge raw AMS data from Kappabridge: '
	write(*,*) '      sample (a10) '
	write(*,*) '      meas(1)  ... meas(5) '
	write(*,*) '      meas(6)  ... meas(10) '
	write(*,*) '      meas(11) ... meas(15) '
	write(*,*) ' with orientation file: '
	write(*,*) '      sample (a10), az, dip, bstrike, bdip '
	write(*,*)
	write(*,*) ' Output files designed for AMSPlot: '
	write(*,*) '      directions.ldgo '
	write(*,*) '      ellipses.ldgo '
	write(*,*) '================================================='
	write(*,*)
	
	write(*,*) ' Critical values for F = 3.48 F12 or F23 = 4.26'
	
	write(*,*)
	write(*,*) ' Enter coordinate system for tensor output (c/g/t) '
	read(*,*) ans
	if(ans.eq."c".or.ans.eq."C") itout = 0
	if(ans.eq."g".or.ans.eq."G") itout = 1
	if(ans.eq."t".or.ans.eq."T") itout = 2
	
	call getazdip(naz,azID,azdata,iaz,cazdata)
	call getrawdat(nraw,rawID,raw)

c===================================================
c    First calculate the tensors for all samples
c    and output a file with the hext statistics
c
c    sample,bulk,sigma,k1,k2,k3,F,F12,F23
c    note that eigenvalues come back k3,k2,k1
c    so swapped on output to file
c===================================================
	
	outfile="hext.stats"
	open(10,file=outfile,status="UNKNOWN")

	do 6 i=1,nraw
	  do 8 j=1,15			! assemble 15 meas
	    x(j)=raw(i,j)
8	  continue
	  call dok15_s(x,a,sigma,bulk)
	  call a2s(a,s)
	  call dohext(9,sigma,s,yell,f,tau,ierr)
	  if(ierr.eq.1)stop
	  
	  do 7 m=1,3			! check values of F tests
	   if(f(m).gt.9999) f(m)=9999.9
7	  continue

	  write(10,99) rawID(i),bulk,sigma,tau(3),tau(2),tau(1),
     &               f(1),f(2),f(3)
	  tens(i,1)=a(1,1)		! tens array now holds 6 
	  tens(i,2)=a(2,2)		! tensor elements, sigma, bulk
	  tens(i,3)=a(3,3)		! index same as for names in
	  tens(i,4)=a(1,2)		! rawID array
	  tens(i,5)=a(2,3)		! tensor in sample coordinates
	  tens(i,6)=a(1,3)
	  tens(i,7)=sigma
	  tens(i,8)=bulk
6	continue
	close(10)

99	format(a10,1x,f10.1,1x,f7.5,1x,3(1x,f7.5),3(1x,f7.2))

c===================================================
c    Get info for site designation, duplicates
c===================================================

	call makesite(width,nraw,rawID,nsite,siteID)
	write(*,*) 
	write(*,*) ' Shall we average duplicate specimens? '
	read(*,*) ans
	if(ans.eq."Y".or.ans.eq."y") then
	  idupe=1
	else
	  idupe=0
	endif

	write(*,*) 
	write(*,*) ' Sample (p), site (P) bootstrap or none (n)? '
	read(*,*) ans
	if(ans.eq."p") then
	  ipar=1
	  write(*,*) ' sample bootstrap selected '
	elseif(ans.eq."P") then
	  ipar=2
	  write(*,*) ' site bootstrap selected '
	elseif(ans.eq."n".or.ans.eq."N") then
	  ipar=3
	  write(*,*) ' no bootstrap '
	endif
	write(*,*)
	
c===================================================
c  Open an output file for principal directions and ellipses
c===================================================

	outfile="directions.ldgo"
	open(5,file=outfile,status="UNKNOWN")
	line="!  "
	write(5,'(a)') line
	line="!   ID      AXIS EIGENVAL       CDECL CINCL
     &  GDECL GINCL  BDECL BINCL"
	write(5,'(a)') line
	line="!__________________________________________
     &__________________________"
	write(5,'(a)') line

	if(ipar.ne.3) then
	  outfile="ellipses.ldgo"
	  open(15,file=outfile,status="UNKNOWN")
	endif
	
	outfile="tensors.dat"
	open(25,file=outfile,status="UNKNOWN")
	
c===================================================
c                 Begin main loop
c===================================================

	do 9 i=1,nsite
	
	 site=siteID(i)				! collect data for site
	 nsamp=0
	 
	 do 13 j=1,nraw
	   if(rawID(j)(1:width).eq.site(1:width))then
	     nsamp=nsamp+1
	     blockID(nsamp)=rawID(j)
	     do 15 k=1,8
	       data(nsamp,k)=tens(j,k)
15	     continue
	   endif
13	 continue

	 if(idupe.eq.1)then			! want to average duplicates
	   call avetensor(width,nsamp,blockID,data)
	 endif

c===================================================
c  Now have nsamp values (averaged if requested)
c  sample IDs in blockID, tensor elements, sigma,
c  bulk susceptibility in data array
c===================================================

	 do 21 j=1,nsamp
	 	 
	  if(iaz.eq.0) then
	    az = cazdata(1)
	    dip = cazdata(2)
	    bstr = cazdata(3)
	    bdip = cazdata(4)
	  else
	    call findaz(blockID(j),naz,azID,azdata,az,dip,bstr,bdip)
	  endif
	  
	  call six2a(data(j,1),data(j,2),data(j,3),data(j,4),data(j,5),
     &             data(j,6),core)  ! rebuild a with averaged values
     	  call a2s(core,s)		! s array for core coordinates
        if(itout.eq.0) then
	    write(25,'(a10,1x,f9.6,3x,6(f9.6,1x))') blockID(j),data(j,7),
     &                             s(1),s(2),s(3),s(4),s(5),s(6)
        endif

	  do 997 mm=1,6
997	  sbs(mm,j)=s(mm) 		! assemble array in core coords
	  sig(j)=data(j,7)
	  
	  call dohext(9,data(j,7),s,yell,f,tau,ierr)
	  if(ierr.eq.1)stop
	  
        do 57 k=1,3
	   yell(k,1)=yell(k,1)/rad		! declination
	   yell(k,2)=yell(k,2)/rad		! inclination
         call flip(yell(k,1),yell(k,2))   ! lower hemisphere
57      continue
	   
	  eigen(1)=tau(3)		! straighten out order of eigenvalues
	  eigen(2)=tau(2)
	  eigen(3)=tau(1)
	  cdecl(1)=yell(3,1)
	  cincl(1)=yell(3,2)
	  cdecl(2)=yell(2,1)
	  cincl(2)=yell(2,2)
	  cdecl(3)=yell(1,1)
	  cincl(3)=yell(1,2)
	  
	  call matrot(1,core,az,dip,geo)
	  call a2s(geo,s)		! s array for geog coordinates
        if(itout.eq.1) then
	    write(25,'(a10,1x,f9.6,3x,6(f9.6,1x))') blockID(j),data(j,7),
     &                             s(1),s(2),s(3),s(4),s(5),s(6)
        endif

	  do 998 mm=1,6
998	  gbs(mm,j)=s(mm)		! assemble array in geog coords

	  call dohext(9,data(j,7),s,yell,f,tau,ierr)
	  if(ierr.eq.1)stop
	  
        do 58 k=1,3
	   yell(k,1)=yell(k,1)/rad
	   yell(k,2)=yell(k,2)/rad
         call flip(yell(k,1),yell(k,2))
58      continue
	   
	  gdecl(1)=yell(3,1)
	  gincl(1)=yell(3,2)
	  gdecl(2)=yell(2,1)
	  gincl(2)=yell(2,2)
	  gdecl(3)=yell(1,1)
	  gincl(3)=yell(1,2)
	  
	  call matrot(2,geo,bstr,bdip,tilt)
	  call a2s(tilt,s)	! s array for tilt coordinates
        if(itout.eq.2) then
	    write(25,'(a10,1x,f9.6,3x,6(f9.6,1x))') blockID(j),data(j,7),
     &                             s(1),s(2),s(3),s(4),s(5),s(6)
        endif
	  
	  do 999 mm=1,6
999	  tbs(mm,j)=s(mm)		! assemble array in tilt coords

	  call dohext(9,data(j,7),s,yell,f,tau,ierr)
	  if(ierr.eq.1)stop
	  
        do 59 k=1,3
	   yell(k,1)=yell(k,1)/rad
	   yell(k,2)=yell(k,2)/rad
         call flip(yell(k,1),yell(k,2))
59      continue
	   
	  bdecl(1)=yell(3,1)
	  bincl(1)=yell(3,2)
	  bdecl(2)=yell(2,1)
	  bincl(2)=yell(2,2)
	  bdecl(3)=yell(1,1)
	  bincl(3)=yell(1,2)
	  
	  do 60 k=1,3
	   write(5,61) blockID(j),k,eigen(k),cdecl(k),cincl(k),gdecl(k),
     &              gincl(k),bdecl(k),bincl(k)
60	  continue
61	format(a10,3x,i1,3x,f8.6,5x,3(2x,f5.1,1x,f5.1))

21	 continue

	if(ipar.eq.3) goto 9	! no bootstrap requested
	
	do 438 m=1,nsamp		! sig array get overwritten in bootstrap
	  sigsave(m)=sig(m)	! so save a version of it for subsequent
438	continue			! calls to bootams

	write(*,*) ' Working on bootstrap for site ', site
	call bootams(ipar,nsamp,sbs,sig,bstrap)
	ans = "s"
	do 441 m=3,1,-1
	write(15,444) site,ans,bstrap(m,1),bstrap(m,2),bstrap(m,3),
     &  bstrap(m,4),bstrap(m,5),bstrap(m,6),bstrap(m,7),
     &  bstrap(m,8),bstrap(m,9),bstrap(m,10)
441	continue

	do 439 m=1,nsamp
	  sig(m)=sigsave(m)
439	continue

	call bootams(ipar,nsamp,gbs,sig,bstrap)
	ans = "g"
	do 442 m=3,1,-1
	write(15,444) site,ans,bstrap(m,1),bstrap(m,2),bstrap(m,3),
     &  bstrap(m,4),bstrap(m,5),bstrap(m,6),bstrap(m,7),
     &  bstrap(m,8),bstrap(m,9),bstrap(m,10)
442	continue

	do 440 m=1,nsamp
	  sig(m)=sigsave(m)
440	continue

	call bootams(ipar,nsamp,tbs,sig,bstrap)
	ans = "t"
	do 443 m=3,1,-1
	write(15,444) site,ans,bstrap(m,1),bstrap(m,2),bstrap(m,3),
     &  bstrap(m,4),bstrap(m,5),bstrap(m,6),bstrap(m,7),
     &  bstrap(m,8),bstrap(m,9),bstrap(m,10)
443	continue

9	continue

	close(5)
	close(15)
	close(25)
	
	write(*,*)
	write(*,*) '====================================================='
	write(*,*) '   Summary sample statistics written to hext.stats '
	write(*,*) '       sample,bulk,sig,tau1,tau2,tau3,F,F12,F23 '
	write(*,*)
	write(*,*) '====================================================='
	write(*,*) '      Eigenvectors written to directions.ldgo '
	write(*,*) '  sample,k,eig,cdecl,cincl,gdecl,gincl,bdecl,bincl '
	write(*,*)
	if(ipar.ne.3) then
	write(*,*) '====================================================='
	write(*,*) '       Ellipse data written to ellipses.ldgo '
	write(*,*) 'samp,coord,eig,sig,D,I,eta,etaD,etaI,zeta,zetaD,zetaI'
	write(*,*) '====================================================='
	write(*,*)
	endif
	write(*,*) '====================================================='
	write(*,*) '          Tensors written to tensor.dat '
	write(*,*) ' samp,sigma,a(1,1),a(2,2),a(3,3),a(1,2),a(2,3),a(1,3)'
	write(*,*) '====================================================='
	write(*,*)
	
444	format(a10,1x,a1,1x,2(f7.5,1x),1x,8(f6.1))
	
	end
c***********************************************************************
	subroutine getazdip(naz,azID,azdata,iaz,cazdata)
c
c Calls no other routines.
c
c Input file: Lamont azdip file (a10,2x,4(1x,f5.1))
c
c	sample ID     Az    Pl    Str    Dip
c	71699-1a     263.7  57.0 152.0   3.0
c	71699-1b     247.4  65.0 152.0   3.0
c	
c Output:
c	azID = contains sample ID (all right padded to 10 characters)
c	azdata = contains az,dip,strike,dip
c	azcount = number of azdip records
c	iaz = 1 if azdip data used
c           0 constant values 
c
c NB: geographic and tilt corrections use Schmidt(1974) rotation
c scheme as implemented in lamont programs. The input information is:
c
c    az = azimuth of direction of drilling
c    pl = dip of direction of drilling
c   str = strike of bedding (90 to left of dip)
c   dip = bedding dip (90 to right of strike)
c

	parameter(maxaz=3000)
	implicit double precision (a-h,o-z)
	character*30 infile
	character*10 azID(maxaz)	! array with IDs from azdip file
	dimension azdata(maxaz,4)	! array with az,dip,bstr,bdip
	dimension cazdata(4)		! array with constant azdip info
	
	write(*,*) ' Enter azdip filename (n for none) '
	read(*,*) infile
	if(infile.eq."N".or.infile.eq."n") then
	  iaz = 0
	  write(*,*) '    Enter constant az,dip,bstr,bdip data '
	  write(*,*) '          0,0,0,0 for ODP cores '
	  write(*,*) '  0,90,0,0 for all coordinates same as core '
	  read(*,*) cazdata(1),cazdata(2),cazdata(3),cazdata(4)
	  write(*,*)
	  goto 15
	else
	  iaz = 1
	  cazdata(1) = 0.
	  cazdata(2) = 90.
	  cazdata(3) = 0.
	  cazdata(4) = 0.
	endif 

	open(10,file=infile,status="OLD")
	
	do 5 i=1,maxaz
	  read(10,'(a10,2x,4(1x,f5.1))',err=9,end=8) azID(i),azdata(i,1),
     &      azdata(i,2),azdata(i,3),azdata(i,4)
5	continue
8	naz = i-1
	write(*,*) naz, " azdip records successfully read "
	goto 11
	
9	write(*,*) ' Error reading azdip file at record: ', i	
11	close(10)

15	return
	end
c***********************************************************************
	subroutine getrawdat(nraw,rawID,raw)
c
c Calls no other routines.
c
c Reads in raw data for from kappabridge program.
c Input file format is:
c
c	record1: sample [labaz labpl strike dip]
c	record2: meas1 meas2 meas3 meas4 meas5
c	record3: meas6 meas7 meas8 meas9 meas10
c	record4: meas11 meas12 meas13 meas14 meas15
c
c blanks between records ignored
c

	parameter(maxraw=3000)
	implicit double precision (a-h,o-z)
	character*10 rawID(maxraw)	! array with IDs assoc with 15 meas
	dimension raw(maxraw,15)	! array with assoc 15 meas
	character*1 space
	character*80 line
	character*30 infile
	space = " "

	write(*,*) ' Enter file name with raw kappabridge data '
	read(*,*) infile
	open(5,file=infile,status="OLD")
	
	nraw = 0
	do 15 i=1,maxraw
16	 read(5,'(a)',err=19,end=20) line	! read sample ID info
	 iflag = index(line,space)
	 if(iflag.eq.1) then			! blank line, reread
	   goto 16
	 else
	   nraw = nraw+1
	   rawID(nraw) = ADJUSTL(line(1:iflag-1))
	 endif
	
       read(5,*,err=19,end=20)(raw(nraw,j),j=1,5)
       read(5,*,err=19,end=20)(raw(nraw,j),j=6,10)
       read(5,*,err=19,end=20)(raw(nraw,j),j=11,15)
15	continue
20	write(*,*) nraw, " records successfully read "
	goto 21

19	write(*,*) ' Error reading raw data file at record: ', i
21	close(5)
	return
	end
c***********************************************************************
	subroutine findaz(samp,naz,azID,azdata,az,dip,bstr,bdip)
c
c Calls no other routines.
c
c  Look for matching sample name in the azID array. First check if there
c  is an exact match and if so use this one. Else check to see if
c  stripping off the last letter or digit provides a match and use this.
c  Note that when two character operands of unequal length are compared
c  the comparison proceeds as though the shorter operand were extended
c  with blank characters to the length of the longer operand. Thus,
c  there is no need to trim the value of the azID string.
c
	parameter(maxaz=3000)
	implicit double precision (a-h,o-z)
	character*10 azID(maxaz)	! array with IDs from azdip file
	dimension azdata(maxaz,4)	! array with az,pl,str,dip
	character*10 samp
	character*1 space
	space = " "
	
	imatch = 0
	ilen = index(samp,space)
	if(ilen.eq.0) ilen=11		! name fills 10 character field
	
	do 30 i=1,naz
	  if(samp(1:ilen-1).eq.azID(i)) then  ! first check for full
	    az = azdata(i,1)			  ! length match
	    dip = azdata(i,2)
	    bstr = azdata(i,3)
	    bdip = azdata(i,4)
	    imatch = 1
	    goto 39
	  endif
30	continue

	if(imatch.eq.0) then			  ! check for n-1 match
	do 31 i=1,naz
	  if(samp(1:ilen-2).eq.azID(i)) then
	    az = azdata(i,1)
	    dip = azdata(i,2)
	    bstr = azdata(i,3)
	    bdip = azdata(i,4)
	    imatch = 1
	    goto 39
	  endif
31	continue
	endif
	
	if(imatch.eq.0) then
	  write(*,*) ' No match found for sample: ', samp
	  az = 0.
	  dip = 0.
	  bstr = 0.
	  bdip = 0.
	endif
	
39	return
	end
c***********************************************************************
	subroutine dok15_s(x,a,s,b)
c
c Calls no other routines.
c
c	calculates least-squares matrix for 15 measurements - from
c	  Jelinek [1976]
c
	implicit double precision (a-h,o-z)
	dimension a(3,3),x(15),del(15)
	
200	do 220 j=1,3
	k=MOD(j,3)+1
	l=MOD(k,3)+1
	R=0.
	
	do 210 i=1,5
	  ji=5*(j-1)+i
	  ki=5*(k-1)+i
	  li=5*(l-1)+i
	  R=R+0.15*(x(ji)+x(li))-0.1*x(ki)
210	continue

c calculate elements of a

	a(j,j)=R+0.25*(x(5*j-2)-x(5*l-2))	   
	a(j,k)=0.25*(-x(5*j-4)+x(5*j-3)-x(5*j-1)+x(5*j))
	a(k,j)=a(j,k)
220	continue

c normalize by trace

	t=(a(1,1)+a(2,2)+a(3,3))
	b=t/3
	do 500 i=1,3
	  do 500 j=1,3
	    a(i,j)=a(i,j)/t
500	continue

c calculate del's

	do 230 j=1,3
	  k=MOD(j,3)+1
	  ji=5*(j-1)
	  del(ji+1)=0.5*(a(j,j)+a(k,k))-a(j,K)
	  del(ji+2)=del(ji+1)+2.*a(j,k)
	  del(ji+3)=a(j,j)
	  del(ji+4)=del(ji+1)
	  del(ji+5)=del(ji+2)
230	continue

	S=0.
	do 240 i=1,15
	  del(i)=x(i)/t-del(i)
	  S=S+del(i)**2
240	continue

	if(S.gt.0)then
	  S=dsqrt(S/9)
	else
	  S=0
	endif
	
	return
	end
c***********************************************************************
	subroutine makesite(width,nraw,rawID,nsite,siteID)
c
c Calls piksrt.
c
c Routine to generate an ordered site list from the raw sample IDs
c 
	parameter(maxaz=3000)
	parameter(maxraw=3000)
	implicit double precision (a-h,o-z)
	character*10 rawID(maxraw)	! array with IDs assoc with 15 meas
	character*10 siteID(maxaz)	! ordered list of unique site IDs
	character*10 oldname,testname
	integer width
	
	write(*,*)
	write(*,*) ' Some representative sample names '
	maxlen=0
	do 40 i=1,nraw
	  ilen=len(trim(rawID(i)))
	  if(ilen.gt.maxlen) then
	    write(*,*) rawID(i)
	    maxlen=ilen
	  endif
40	continue

	write(*,*)
	write(*,*) ' Enter search width for unique site IDs '
	read(*,*) width
	
	siteID(1)=adjustl(rawID(1)(1:width))
	nsite=1
	oldname=siteID(1)
	
	do 45 i=2,nraw
	  testname=adjustl(rawID(i)(1:width))
	  match=0
	  do 43 j=1,nsite			! look through existing list
	   if(testname.eq.siteID(j)) match=1
43	  continue
	  if(match.eq.0) then		! must be new name
	    nsite=nsite+1
	    siteID(nsite)=testname
	    oldname=testname
	    match=0
	  endif
45	continue

	call piksrt(nsite,siteID)	! sort the list of unique IDs
	
	return
	end
c***********************************************************************
	subroutine piksrt(n,arr)
c 
c Calls no other routines.
c
c Straight insertion sorting routine from Numerical Recipes (p. 227).
c NB: N^2 routine so don't use for big N.
c
	parameter(maxaz=3000)
	implicit double precision (a-h,o-z)
	character*10 arr(maxaz)		! ordered list of unique site IDs
	character*10 a
	
	do 12 j=2,n
	  a=arr(j)
	  do 11 i=j-1,1,-1
	    if(arr(i).le.a)goto 10
	    arr(i+1)=arr(i)
11	  continue
	  i=0
10	  arr(i+1)=a
12	continue
	return
	end
c***********************************************************************
	subroutine avetensor(width,nsamp,blockID,data)
c 
c Calls no other routines.
c
c Averages duplicate specimen tensors. Original array overwritten.
c

	implicit double precision (a-h,o-z)
	integer width			! search width for site IDs
	character*10 blockID(100)	! IDs for data block from one site
	dimension data(100,8)		! associated sample tensor data
	character*10 aveID(100)		! IDs for specimen averaged data
	dimension ave(100,8)		! associated tensor data
	character*10 unique(100)	! array of unique names
	character*10 samp,oldsamp
	dimension tmp(8)

c first it's useful to have an ordered list of the unique names

	nu=1					! number of unique names
	samp=adjustl(blockID(1)(1:width+1))
	oldsamp=samp
	unique(nu)=samp
	
	do 5 j=2,nsamp
	  samp=adjustl(blockID(j)(1:width+1))
	  match=0
	  do 6 k=1,nu
	    if(samp.eq.unique(k)) match=1	      
6	  continue
	  if(match.eq.0) then		! must be new name
	    nu=nu+1
	    unique(nu)=samp
	    oldsamp=samp
	    match=0
	  endif
5	continue

	call piksrt(nu,unique)	      ! sort the list of unique IDs

c now we can proceed to average the data

	do 7 i=1,nu
	  aveID(i)=unique(i)
	  nn=0
	  do 8 k=1,8
	    tmp(k)=0.			! make sure average start at 0
8	  continue

	  do 9 j=1,nsamp
	    samp=adjustl(blockID(j)(1:width+1))
	    if(samp.eq.aveID(i)) then
	      nn=nn+1
		do 12 m=1,8
		  tmp(m)=tmp(m)+data(j,m)
12		continue
	    end if
9	  continue

	do 13 mm=1,8
	  ave(i,mm)=tmp(mm)/nn
13	continue

7	continue

c finally overwrite the initial arrays

	nsamp=nu
	do 15 i=1,nsamp
	  blockID(i)=aveID(i)
	  do 20 j=1,8
	    data(i,j)=ave(i,j)
20	  continue
15	continue

	return
	end
c***********************************************************************
	subroutine dohext(nf,sig,s,yell,f,tau,ierr) 
c
c Calls fcalc,ql
c
c  subroutine to compute mean anisotropic susceptibility for a number of
c  measurements, anisotropy parameters for each measurement and mean
c  and uncertainty estimates under linear propagation assumption.
c
	implicit double precision (a-h,o-z)
	dimension a(3,3),ei(3),work(3,3)
	dimension s(6),f(3),tau(3)
	dimension yell(3,8)
	ierr=0
	call fcalc(nf,fstat,ierr)
	if(ierr.eq.1)return
	do 220 k=1,3
 220	a(k,k)=s(k) 
	a(1,2)=s(4)
	a(2,3)=s(5)
	a(1,3)=s(6)
	a(2,1)=a(1,2)
	a(3,2)=a(2,3)
	a(3,1)=a(1,3)
	call ql(3, 3, a,ei, work, ierr)
	do 88 j=1,3
 88	tau(j)=ei(j)
	do 1000 i=1,3
	 k=mod(i+1,3)
	 if(k.eq.0)k=3
	 l=mod(i+2,3)
	 if(l.eq.0)l=3
c  summarize results
	 yell(i,3)=datan2(dsqrt(2*fstat)*sig,2*abs(ei(i)-ei(k)))
	 yell(i,6)=datan2(dsqrt(2*fstat)*sig,2*abs(ei(i)-ei(l)))
	 yell(i,1)=datan2(a(2,i),a(1,i))
	 yell(i,2)=dasin(a(3,i))
	 yell(i,4)=datan2(a(2,k),a(1,k))
	 yell(i,5)=dasin(a(3,k))
	 yell(i,7)=datan2(a(2,l),a(1,l))
	 yell(i,8)=dasin(a(3,l))
	 if (yell(i,3).gt.yell(i,6)) then
	   tmp1=yell(i,3)
	   tmp2=yell(i,4)
	   tmp3=yell(i,5)
	   yell(i,3)=yell(i,6)
	   yell(i,4)=yell(i,7)
	   yell(i,5)=yell(i,8)
	   yell(i,6)=tmp1 
	   yell(i,7)=tmp2 
	   yell(i,8)=tmp3 
	 endif
1000	continue
	bulk=(s(1)+s(2)+s(3))/3
	f(1)=ei(1)**2+ei(2)**2+ei(3)**2-3*(bulk**2)
        f(1)=0.4*f(1)/(sig**2)
        f(2)=0.5*((ei(3)-ei(2))/sig)**2
        f(3)=0.5*((ei(2)-ei(1))/sig)**2
	return
	end
c***********************************************************************
	subroutine fcalc(nf,f,ierr)
c
c Calls no other routines.
c
c	looks up f from f tables 
c
	implicit double precision (a-h,o-z)
	ierr=0
	if(nf.lt.2) then
	  write(*,*)'doesnt work on this few samples'
	  ierr=1
	  return
	endif
	if(nf.ge.2)f=19
	if(nf.ge.3)f=9.55
	if(nf.ge.4)f=6.94
	if(nf.ge.5)f=5.79
	if(nf.ge.6)f=5.14
	if(nf.ge.7)f=4.74
	if(nf.ge.8)f=4.46
	if(nf.ge.9)f=4.26
	if(nf.ge.10)f=4.1
	if(nf.ge.11)f=3.98
	if(nf.ge.12)f=3.89
	if(nf.ge.13)f=3.81
	if(nf.ge.14)f=3.74
	if(nf.ge.15)f=3.68
	if(nf.ge.16)f=3.63
	if(nf.ge.17)f=3.59
	if(nf.ge.18)f=3.55
	if(nf.ge.19)f=3.52
	if(nf.ge.20)f=3.49
	if(nf.ge.21)f=3.47
	if(nf.ge.22)f=3.44
	if(nf.ge.23)f=3.42
	if(nf.ge.24)f=3.4
	if(nf.ge.25)f=3.39
	if(nf.ge.26)f=3.37
	if(nf.ge.27)f=3.35
	if(nf.ge.28)f=3.34
	if(nf.ge.29)f=3.33
	if(nf.ge.30)f=3.32
	if(nf.ge.32)f=3.29
	if(nf.ge.34)f=3.28
	if(nf.ge.36)f=3.26
	if(nf.ge.38)f=3.24
	if(nf.ge.40)f=3.23
	if(nf.ge.60)f=3.15
	if(nf.ge.120)f=3.07
	if(nf.ge.500)f=3
	return
	end
c***********************************************************************
      subroutine ql(nm, n, a, d, e, ierr)
	implicit double precision (a-h,o-z)
c$$$$ calls tred2, tql2
c  using  eispack  routines tred2, tql2,  solves the symmetric
c  eigenvalue-eigenvector problem for a real matrix.
c  on input
c  nm   row dimension of the symmetric array  a  in the caller.
c  n    order of the array (<= nm)
c  a    the real symmetric array to be treated
c  e     a working array at least  n  long
c
c  on output
c  d    the array of eigenvalues ascending
c  a    the corresponding array of eigenvectors, with row
c       dimension  nm.  original  a  is overwritten.
c  ierr 0  if all's well.
c       j  if eigenvalue number j and above not found.
c
c
      dimension a(nm,*), d(*), e(*)
      call tred2(nm, n, a, d, e, a)
c
      call tql2(nm, n, d, e, a, ierr)
      return
      end
c***********************************************************************
c
      subroutine tred2(nm, n, a, d, e, z)
	implicit double precision (a-h,o-z)
c$$$$ calls no other routines
c
      dimension a(nm,n),d(n),e(n),z(nm,n)      
      double precision f,g,h,hh,scale   
c
c     this subroutine is a translation of the algol procedure tred2,
c     num. math. 11, 181-195(1968) by martin, reinsch, and wilkinson.
c     handbook for auto. comp., vol.ii-linear algebra, 212-226(1971).
c
c     this subroutine reduces a real symmetric matrix to a
c     symmetric tridiagonal matrix using and accumulating
c     orthogonal similarity transformations.
c
c     on input:
c
c        nm must be set to the row dimension of two-dimensional
c          array parameters as declared in the calling program
c          dimension statement;
c
c        n is the order of the matrix;
c
c        a contains the real symmetric input matrix.  only the
c          lower triangle of the matrix need be supplied.
c
c     on output:
c
c        d contains the diagonal elements of the tridiagonal matrix;
c
c        e contains the subdiagonal elements of the tridiagonal
c          matrix in its last n-1 positions.  e(1) is set to zero;
c
c        z contains the orthogonal transformation matrix
c          produced in the reduction;
c
c        a and z may coincide.  if distinct, a is unaltered.
c
c     questions and comments should be directed to b. s. garbow,
c     applied mathematics division, argonne national laboratory
c
c     ------------------------------------------------------------------
c
      do 100 i = 1, n
c
         do 100 j = 1, i
            z(i,j) = a(i,j)
  100 continue
c
      if (n .eq. 1) go to 320
c     :::::::::: for i=n step -1 until 2 do -- ::::::::::
      do 300 ii = 2, n
         i = n + 2 - ii
         l = i - 1
         h = 0.0d0
         scale = 0.0d0
         if (l .lt. 2) go to 130
c     :::::::::: scale row (algol tol then not needed) ::::::::::
         do 120 k = 1, l
  120    scale = scale + abs(z(i,k))
c
         if (scale .ne. 0.0d0) go to 140
  130    e(i) = z(i,l)
         go to 290
c
  140    do 150 k = 1, l
            z(i,k) = z(i,k) / scale
            h = h + z(i,k) * z(i,k)
  150    continue
c
         f = z(i,l)
	if(h.lt.0)then
	write(14,*)'problem in tred2'
	endif
         g = -sign(dsqrt(h),f)
         e(i) = scale * g
         h = h - f * g
         z(i,l) = f - g
         f = 0.0d0
c
         do 240 j = 1, l
            z(j,i) = z(i,j) / h
            g = 0.0d0
c     :::::::::: form element of a*u ::::::::::
            do 180 k = 1, j
  180       g = g + z(j,k) * z(i,k)
c
            jp1 = j + 1
            if (l .lt. jp1) go to 220
c
            do 200 k = jp1, l
  200       g = g + z(k,j) * z(i,k)
c     :::::::::: form element of p ::::::::::
  220       e(j) = g / h
            f = f + e(j) * z(i,j)
  240    continue
c
         hh = f / (h + h)
c     :::::::::: form reduced a ::::::::::
         do 260 j = 1, l
            f = z(i,j)
            g = e(j) - hh * f
            e(j) = g
c
            do 260 k = 1, j
               z(j,k) = z(j,k) - f * e(k) - g * z(i,k)
  260    continue
c
  290    d(i) = h
  300 continue
c
  320 d(1) = 0.0d0
      e(1) = 0.0d0
c     :::::::::: accumulation of transformation matrices ::::::::::
      do 500 i = 1, n
         l = i - 1
         if (d(i) .eq. 0.0d0) go to 380
c
         do 360 j = 1, l
            g = 0.0d0
c
            do 340 k = 1, l
  340       g = g + z(i,k) * z(k,j)
c
            do 360 k = 1, l
               z(k,j) = z(k,j) - g * z(k,i)
  360    continue
c
  380    d(i) = z(i,i)
         z(i,i) = 1.0d0
         if (l .lt. 1) go to 500
c
         do 400 j = 1, l
            z(i,j) = 0.0d0
            z(j,i) = 0.0d0
  400    continue
c
  500 continue
c
      return
      end
c***********************************************************************
c
      subroutine tql2(nm, n, d, e, z, ierr)
	implicit double precision (a-h,o-z)
c$$$$ calls no other routines
c
      dimension d(n),e(n),z(nm,n)                                
      double precision b,c,f,g,h,p,r,s,machep                          
c
c     this subroutine is a translation of the algol procedure tql2,
c     num. math. 11, 293-306(1968) by bowdler, martin, reinsch, and
c     wilkinson.
c     handbook for auto. comp., vol.ii-linear algebra, 227-240(1971).
c
c     this subroutine finds the eigenvalues and eigenvectors
c     of a symmetric tridiagonal matrix by the ql method.
c     the eigenvectors of a full symmetric matrix can also
c     be found if  tred2  has been used to reduce this
c     full matrix to tridiagonal form.
c
c     on input:
c
c        nm must be set to the row dimension of two-dimensional
c          array parameters as declared in the calling program
c          dimension statement;
c
c        n is the order of the matrix;
c
c        d contains the diagonal elements of the input matrix;
c
c        e contains the subdiagonal elements of the input matrix
c          in its last n-1 positions.  e(1) is arbitrary;
c
c        z contains the transformation matrix produced in the
c          reduction by  tred2, if performed.  if the eigenvectors
c          of the tridiagonal matrix are desired, z must contain
c          the identity matrix.
c
c      on output:
c
c        d contains the eigenvalues in ascending order.  if an
c          error exit is made, the eigenvalues are correct but
c          unordered for indices 1,2,...,ierr-1;
c
c        e has been destroyed;
c
c        z contains orthonormal eigenvectors of the symmetric
c          tridiagonal (or full) matrix.  if an error exit is made,
c          z contains the eigenvectors associated with the stored
c          eigenvalues;
c
c        ierr is set to
c          zero       for normal return,
c          j          if the j-th eigenvalue has not been
c                     determined after 30 iterations.
c
c     ------------------------------------------------------------------
c
c     :::::::::: machep is a machine dependent parameter specifying
c                the relative precision of floating point arithmetic.
c                machep = 16.0d0**(-13) for long form arithmetic
c                on s360 ::::::::::
      data machep/1.421d-14/                                          
c
      ierr = 0
      if (n .eq. 1) go to 1001
c
      do 100 i = 2, n
  100 e(i-1) = e(i)
c
      f = 0.0d0
      b = 0.0d0
      e(n) = 0.0d0
c
      do 240 l = 1, n
         j = 0
         h = machep * (abs(d(l)) + abs(e(l)))
         if (b .lt. h) b = h
c     :::::::::: look for small sub-diagonal element ::::::::::
         do 110 m = l, n
            if (abs(e(m)) .le. b) go to 120
c     :::::::::: e(n) is always zero, so there is no exit
c                through the bottom of the loop ::::::::::
  110    continue
c
  120    if (m .eq. l) go to 220
  130    if (j .eq. 30) go to 1000
         j = j + 1
c     :::::::::: form shift ::::::::::
         l1 = l + 1
         g = d(l)
         p = (d(l1) - g) / (2.0d0 * e(l))
         r = dsqrt(p*p+1.0d0)
         d(l) = e(l) / (p + sign(r,p))
         h = g - d(l)
c
         do 140 i = l1, n
  140    d(i) = d(i) - h
c
         f = f + h
c     :::::::::: ql transformation ::::::::::
         p = d(m)
         c = 1.0d0
         s = 0.0d0
         mml = m - l
c     :::::::::: for i=m-1 step -1 until l do -- ::::::::::
         do 200 ii = 1, mml
            i = m - ii
            g = c * e(i)
            h = c * p
            if (abs(p) .lt. abs(e(i))) go to 150
            c = e(i) / p
            r = dsqrt(c*c+1.0d0)
            e(i+1) = s * p * r
            s = c / r
            c = 1.0d0 / r
            go to 160
  150       c = p / e(i)
            r = dsqrt(c*c+1.0d0)
            e(i+1) = s * e(i) * r
            s = 1.0d0 / r
            c = c * s
  160       p = c * d(i) - s * g
            d(i+1) = h + s * (c * g + s * d(i))
c     :::::::::: form vector ::::::::::
            do 180 k = 1, n
               h = z(k,i+1)
               z(k,i+1) = s * z(k,i) + c * h
               z(k,i) = c * z(k,i) - s * h
  180       continue
c
  200    continue
c
         e(l) = s * p
         d(l) = c * p
         if (abs(e(l)) .gt. b) go to 130
  220    d(l) = d(l) + f
  240 continue
c     :::::::::: order eigenvalues and eigenvectors ::::::::::
      do 300 ii = 2, n
         i = ii - 1
         k = i
         p = d(i)
c
         do 260 j = ii, n
            if (d(j) .ge. p) go to 260
            k = j
            p = d(j)
  260    continue
c
         if (k .eq. i) go to 300
         d(k) = d(i)
         d(i) = p
c
         do 280 j = 1, n
            p = z(j,i)
            z(j,i) = z(j,k)
            z(j,k) = p
  280    continue
c
  300 continue
c
      go to 1001
c     :::::::::: set error -- no convergence to an
c                eigenvalue after 30 iterations ::::::::::
 1000 ierr = l
	write(*,*)'No convergence after 30 iterations'
 1001 return
      end
c
c
c***********************************************************************
	subroutine dotpr_xyz(t,p,r,x,y,z)
c
c	calls no other routines
c	 takes phi, theta, (in radians) and r, converts to x,y,z
c
	implicit double precision (a-h,o-z)
	x=r*dsin(t)*dcos(p)
	y=r*dsin(t)*dsin(p)
	z=r*dcos(t)
	return
	end
c***********************************************************************
	subroutine flip(dec,dip)
c
c calls no other routines
c
c	puts dec inc in lower hemisphere
c
	implicit double precision (a-h,o-z)
	if(dip.lt.0)then
	 dip=-dip
	 dec=dec-180
	endif
	if(dec.lt.0) dec=dec+360
	return
	end
c***********************************************************************
	subroutine six2a(x1,x2,x3,x4,x5,x6,a)
c
c calls no other routines
c
c takes 3,3 matrix a and assembles s array (k11,k22,k33,k12,k23,k13)
c
	implicit double precision (a-h,o-z)
	dimension a(3,3)
	
	  a(1,1)=x1	
	  a(2,2)=x2
	  a(3,3)=x3
	  a(1,2)=x4
	  a(2,3)=x5
	  a(1,3)=x6
	  a(2,1)=a(1,2)
	  a(3,2)=a(2,3)
	  a(3,1)=a(1,3)
	  
	return
	end
c***********************************************************************
	subroutine a2s(a,s)
c
c calls no other routines
c
c takes s array (k11,k22,k33,k12,k23,k13) and assembles 3,3 matrix a 
c
	implicit double precision (a-h,o-z)
	dimension a(3,3),s(6)

	  s(1)=a(1,1)		
	  s(2)=a(2,2)
	  s(3)=a(3,3)
	  s(4)=a(1,2)
	  s(5)=a(2,3)
	  s(6)=a(1,3)
	
	return
	end
c***********************************************************************
	subroutine  bootams(ipar,nsamp,bs,sig,bstrap)
c
c calls sitesig,s2a,apseudo,evec,kentpar,dostat,flip
c
c Subroutine to calculate confidence intervals for tensor data
c algorithm described in Constable and Tauxe, 1990. Note that some of 
c array names have been modified since designations were already used.
c
c sample bootstrap (-p)    ipar = 1
c site bootstrap (-P)      ipar = 2
c
c Input: bs    array six tensor elements for block of data
c              x11,x22,x33,x12,x23,x13
c	   sig   array with correponding sigma values
c	   nsamp number of samples in the data block
c
c Output: bootstrap error stats as three sets of 
c         tau,stdev,dec,inc,eta,dec,inc,zeta,dec,inc
c

	implicit double precision (a-h,o-z)
	parameter(nb=1000)

	dimension bs(6,nb),ps(6,nb),sig(nb+1)
	dimension par(6),p(nb+1),t(nb+1)
	dimension v(3,3,nb+1)
	dimension bei(3,nb+1)
	dimension a(3,3),e(3)
	dimension tmpx(3),tmpy(3)
	dimension bstrap(3,10)
	
	pi=2.0*dasin(1.0d0)
	rad=pi/180.
	     
c This replaces subroutine adread, since arrays bs,sig are passed.
c Make sure incoming data are normalized by trace. If ipar=2 (option P)
c then calculate standard deviation for whole site and put in sig(1).
c If ipar=1 (option p, sample bootstrap) then normalize sig(i) as well.

	do 10 i=1,nsamp
         trace=(bs(1,i)+bs(2,i)+bs(3,i))
         do 22 j=1,6
           bs(j,i)=bs(j,i)/trace
	     if(ipar.eq.1)sig(i)=sig(i)/trace
22       continue
10	continue

	if(ipar.eq.2)call sitesig(bs,nsamp,sig(1))

c Calculate mean eigenparameters for the data, store result in
c first slot of v

	call s2a(bs,nsamp,a,e)
	do 23 kk=1,3
	 bei(kk,1)=e(kk)
	  do 23 jj=1,3
23	  v(kk,jj,1)=a(kk,jj)

c Generate nb bootstrap pseudosamples drawn using subroutine apseudo
c and put eigen parameters in v and bei

 	do 555 ib=2,nb+1
	  call apseudo(ipar,sig,nsamp,bs,ps)
	  call s2a(ps,nsamp,a,e)
	do 33 kk=1,3
	bei(kk,ib)=e(kk)
	do 33 jj=1,3
	
33	v(kk,jj,ib)=a(kk,jj)
        do 5 j=1,3
         do 666 mmm=1,3
          tmpx(mmm)=v(mmm,j,1)
          tmpy(mmm)=v(mmm,j,ib)
666     continue
          x=dotty(tmpx,tmpy)
          if(x.lt.0.)then
           do 6 mm=1,3
             v(mm,j,ib)=-v(mm,j,ib)
6         continue
          endif
5       continue
555	continue
 
c	calculate kentpars for each axis

	do 200 j=3,1,-1
	 do 55 kk=1,3
	 do 55 jj=1,3
 55	 a(kk,jj)=v(kk,jj,1)
	call evec(a,j,pbar,tbar)
	do 110 i=2,nb+1

c	first calculate eigenvectors phi,theta
	 do 44 kk=1,3
	 do 44 jj=1,3
 44	  a(kk,jj)=v(kk,jj,i)
	 call evec(a,j,p(i-1),t(i-1))
 110	 continue
    	 call kentpar(nb,p,t,pbar,tbar,par)

c	calculate stdev of eigenvalue - reuse sigma
	
	 do 50 kk=1,nb
	 sig(kk)=bei(j,kk+1)
 50	 continue
	 call dostat(nb,sig,xbar,sum,stdev)
	 dec=pbar/rad
	 dip=90-tbar/rad
	 call flip(dec,dip)
	 zeta=par(1)/rad
	 zetad=par(2)/rad
	 zetai=90-par(3)/rad
	 call flip(zetad,zetai)
	 eta=par(4)/rad
	 etad=par(5)/rad
	 etai=90-par(6)/rad
	 call flip(etad,etai)
c 	 write(*,4)bei(j,1),stdev,dec,dip,eta,etad,etai,zeta,zetad,zetai
	 
	 bstrap(j,1)=bei(j,1)
	 bstrap(j,2)=stdev
	 bstrap(j,3)=dec
	 bstrap(j,4)=dip
	 bstrap(j,5)=eta
	 bstrap(j,6)=etad
	 bstrap(j,7)=etai
	 bstrap(j,8)=zeta
	 bstrap(j,9)=zetad
	 bstrap(j,10)=zetai
	 
 200	continue
 4      format(2(f7.5,1x),1x,8(f6.1,1x))
	end

c***********************************************************************
	subroutine sitesig(bs,npts,sig)
c
c calls no other routines.
c
c	calculates sigma for whole site using Hext method
c	based on s_hext.f
c
	implicit double precision (a-h,o-z)
	dimension bs(6,*), d(1000,6),avd(6)

	s0=0
	do 20 j=1,6	
20	avd(j)=0
 	do 100 i=1,npts
	 do 34 j=1,6
34      d(i,j)=bs(j,i)
         d(i,4)=d(i,4)+.5*(d(i,1)+d(i,2))
         d(i,5)=d(i,5)+.5*(d(i,2)+d(i,3))
         d(i,6)=d(i,6)+.5*(d(i,1)+d(i,3))
         do 8 j=1,6
          avd(j)=avd(j) + d(i,j)
8        continue
100    continue 

c	calculate sigma 

200	nf=(npts-1)*6
	do 250 j=1,6
	 avd(j)=avd(j)/float(npts)
250	continue
	do 454 i=1,npts
        do 454 j=1,6
	 s0=s0+(d(i,j)-avd(j))**2	
454	continue
	sig=dsqrt(s0/float(nf))
	
	return
	end
c***********************************************************************
	subroutine s2a(s,npts,a,ei)
c
c calls ql
c
c	calculates a matrix for all the s data
c
	implicit double precision (a-h,o-z)
	dimension s(6,*),a(3,3),ei(3),gamma(3,3)

      pi=2.0*dasin(1.0d0)
      rad=pi/180

c  Compute result for actual mean data values
c  First set up average matrix in a(i,j) then do eigenvalue
c  decomposition in place

c  Initialize a(j,k) to zero
          do 225 j=1,3
          do 225 k=1,3
225       a(j,k)=0.

        do 224 i=1,npts
          do 223 k=1,3
223       a(k,k)=a(k,k) + s(k,i)/float(npts)
          a(2,1)=a(2,1) + s(4,i)/float(npts)
          a(1,2)=a(1,2) + s(4,i)/float(npts)
          a(2,3)=a(2,3) + s(5,i)/float(npts)
          a(3,2)=a(3,2) + s(5,i)/float(npts)
          a(1,3)=a(1,3) + s(6,i)/float(npts)
          a(3,1)=a(3,1) + s(6,i)/float(npts)
224     continue
c
c  find eigenparams
        call ql(3,3,a,ei,gamma,ierr)
        if (ierr.ne.0)then
          print *,'problem in eigenparameter computation'
          stop
        endif
c  put eigenvectors in same hemisphere as mean
	return
	end
c***********************************************************************
	subroutine apseudo(ipar,sig,n,s,ps)
c
c calls no other routines.
c
	implicit double precision (a-h,o-z)
	dimension s(6,*),ps(6,*),sig(*)
	
 	do 20 i=1,n
c	
c	pick a random number ranging from 1 to n
c
  	R1=RAND(0)
	j=1+int(R1*float(n-1))
c
c
	do 10 k=1,6
	if(ipar.eq.0)ps(k,i)=s(k,j)
	if(ipar.eq.1)ps(k,i)=s(k,j)+sig(j)*gaussdev(0)
	if(ipar.eq.2)ps(k,i)=s(k,j)+sig(1)*gaussdev(0)
 10	continue
c
 20	continue
	return
	end
c***********************************************************************
      function gaussdev(seed) 
c
c	following algorithm of Numerical Recipes Press et al. 1986
	implicit double precision (a-h,o-z)
      iflag=0
      if(iflag.eq.0) then
1       r1=2.*rand(seed)-1.
        r2=2.*rand(seed)-1.
        r=r1**2+r2**2
        if(r.ge.1.)goto 1
        fac=dsqrt(-2.*log(r)/r)
        g=r1*fac
        gaussdev=r2*fac
        iflag=1
      else
        gaussdev=g
        iflag=0
      endif
      return
      end
c***********************************************************************
	function dotty(x,y)
c  dot product of 3-vectors x and y
	implicit double precision (a-h,o-z)
	dimension x(3),y(3)
	dotty=x(1)*y(1) + x(2)*y(2) + x(3)*y(3)
	return
	end
c***********************************************************************
	subroutine evec(a,k,p,t)
c
c calls doxyz_tpr
c
	implicit double precision (a-h,o-z)
	dimension a(3,3)
	  x=a(1,k)
	  y=a(2,k)
	  z=a(3,k)
c
c	convert to phi,theta
c
	  call doxyz_tpr(x,y,z,t,p,r)
 15	 continue
	return
	end
c***********************************************************************
	subroutine dostat(n,a,xmean,sum,stdev)
c
c calls no other routines.
c
	implicit double precision (a-h,o-z)
	dimension a(*)
	
	sum=0
	xsum=0
	d=0
	do 150 i=1,n
	sum=sum+a(i)
 150	continue
	xmean=sum/float(n)
	do 200 i=1,n
 200 	d=d+(a(i)-xmean)**2	
	stdev=(1/(float(n)-1))*d
	stdev=dsqrt(stdev)
	return
	end
c***********************************************************************
	subroutine kentpar(n,phi,theta,pbar,tbar,par)
c
c calls calct,dotpr_xyz,doxyz_tpr
c

	implicit double precision (a-h,o-z)
	dimension phi(*),theta(*),par(*)
	dimension x(3),xg(3,10000),h(3,3),b(3,3),w(3,3)
     	dimension gam(3,3) 
	dimension t(3,3)
      pi=2.0*dasin(1.0d0)
      rad=pi/180
	
c
c	calculate orientation matrix of phi and theta
c
	call calct(phi,theta,t,n)
c
c  Find rotation matrix H
	h(1,1)=dcos(tbar)*dcos(pbar)
	h(1,2)=-dsin(pbar)
	h(1,3)=dsin(tbar)*dcos(pbar)
	h(2,1)=dcos(tbar)*dsin(pbar)
	h(2,2)=dcos(pbar)
	h(2,3)=dsin(pbar)*dsin(tbar)
	h(3,1)=-dsin(tbar)
	h(3,2)=0.
	h(3,3)=dcos(tbar)
c
c  Compute B=H'TH
c
	do 10 i=1,3
	do 10 j=1,3
	w(i,j)=0.
	do 10 k=1,3
	  w(i,j)=w(i,j)+t(i,k)*h(k,j)
10	continue
	do 15 i=1,3
	do 15 j=1,3
	b(i,j)=0.
	do 15 k=1,3
	  b(i,j)=b(i,j) + h(k,i)*w(k,j)
15	continue
c
c  Choose a rotation w about north pole to diagonalize upper part of B
c
	psi=0.5*datan(2.*b(1,2)/(b(1,1)-b(2,2)))
	w(1,1)=dcos(psi)
	w(1,2)=-dsin(psi)
	w(1,3)=0.
	w(2,1)=dsin(psi)
	w(2,2)=dcos(psi)
	w(2,3)=0.
	w(3,1)=0.
	w(3,2)=0.
	w(3,3)=1.
	do 20 i=1,3
	do 20 j=1,3
	  gam(i,j)=0.
	  do 20 k=1,3
	  gam(i,j)=gam(i,j) + h(i,k)*w(k,j)
20	continue
c
c  Convert data to x,y,z, rotate to standard frame xg
c
	do 25 i=1,n
	 call dotpr_xyz(theta(i),phi(i),1.,x(1),x(2),x(3))
	  do 25 k=1,3
	  xg(k,i)=0.
	  do 25 j=1,3
	  xg(k,i)=xg(k,i) + gam(j,k)*x(j)
25	continue
c
c  Compute asymptotic ellipse parameters
c
	xmu=0.
	sigma1=0.
	sigma2=0.
	do 30 i=1,n
	xmu=xmu + xg(3,i)
	sigma1=sigma1 + xg(1,i)*xg(1,i)
	sigma2=sigma2 + xg(2,i)*xg(2,i)
30	continue
	xmu=xmu/float(n)
	sigma1=sigma1/float(n)
	sigma2=sigma2/float(n)
	iswitch=0
	if(sigma1.gt.sigma2) then
	       iswitch=1
		tmp=sigma1
		sigma1=sigma2
		sigma2=tmp
	endif
	g=-2.0*log(.05)/(xmu*xmu)
	if(dsqrt(sigma1*g).lt.1)zeta=dasin(dsqrt(sigma1*g))
 	if(dsqrt(sigma2*g).lt.1)eta=dasin(dsqrt(sigma2*g))
 	if(dsqrt(sigma1*g).ge.1.0)zeta=pi/2
 	if(dsqrt(sigma2*g).ge.1.0)eta=pi/2
c
c  Convert Kent parameters to directions etc
	x1=gam(1,2)
	y=gam(2,2)
	z=gam(3,2)
	call doxyz_tpr(x1,y,z,t_zeta,p_zeta,dum)
	x1=gam(1,1)
	y=gam(2,1)
	z=gam(3,1)
	call doxyz_tpr(x1,y,z,t_eta,p_eta,dum)
	par(4)=zeta
	par(1)=eta
	if(iswitch.eq.0)then
	 par(2)=p_zeta
	 par(3)=t_zeta
	 par(5)=p_eta
	 par(6)=t_eta
	else
	 par(2)=p_eta
	 par(3)=t_eta
	 par(5)=p_zeta
	 par(6)=t_zeta
	endif
	return
	end
c
c***********************************************************************
	subroutine calct(phi,theta,t,n)
c
c call dotpr_xyz,tmatrix
c
	implicit double precision (a-h,o-z)
	dimension phi(*),theta(*),x(3,1000)
	dimension t(3,3)
	
	do 10 i=1,n
	call dotpr_xyz(theta(i),phi(i),1.0d0,x(1,i),x(2,i),x(3,i))
 10	continue
	call tmatrix(n,x,t)
	do 20 i=1,3
	do 20 j=1,3
	  t(i,j)=t(i,j)/float(n)
 20	continue
	return
	end
c***********************************************************************
	subroutine tmatrix(n,x,t)
c
c calls no other routines.
c
	implicit double precision (a-h,o-z)
	dimension x(3,*)
	dimension t(3,3)
c
c	initialize t matrix
c
	do 10 i=1,3
	do 10 j=1,3
	 t(i,j)=0
10	continue
c
c	do sums of squares and products
c
	do 20 i=1,n
	 do 20 j=1,3
	  do 20 k=1,3
	   t(j,k)=t(j,k)+x(j,i)*x(k,i)
 20	continue
	return
	end
c***********************************************************************
	subroutine doxyz_tpr(x,y,z,t,p,r)
c
c calls no other routines.
c
c
c Takes x,y,z components and returns theta (t) and phi (p)
c in radians.
c
c Note that this routine forces theta (colatitude) to be between
c 180 and -180.

	implicit double precision (a-h,o-z)
	pi=2.0*dasin(1.0d0)
	r=dsqrt(x*x+y*y+z*z)
	t=dacos(z/r)
   	if (x.eq.0.0) then
        if (y.lt.0) then
          p= 3*pi/2
        else 
         p= pi/2     
	  endif
	  return
	endif
	
      p = (datan(y/x))
      if (x.lt.0) then
        p = p + pi
      endif
	if (p.lt.0) then
	  p = p+2*pi
	endif
	return
 100	end               
c***********************************************************************
	subroutine matrot(ip,a,theta,phi,b)
c
c Routine to rotate matrix a, using conventions in calct, to matrix b
c Input: theta,phi (degrees)
c	 matrix a to be rotated
c	 ip = 1 (sample to geographic)
c	 ip = 2 (geographic to bedding corrected)
c	 ip = 3 (plunge correction)
c
	implicit double precision (a-h,o-z)
	dimension a(3,3),t(3,3),b(3,3)

	call calcrot(ip,theta,phi,t)

c
c Rotation of tensor A can be accomplished by:
c               Arot = (T*A)*Tinv
c where T is the rotation matrix, given here by routine calct
c
c Since Ttrans = Tinv for orthogonal eigenvectors normalized to unit
c length (see eg. Shearer, 1999), this can be written as
c               Arot = (T*A)*Ttran
c or
c               Arot(i,j)=T(i,k)*A(k,l)*T(j,l) 
c
	dum=0			
	do 15 i=1,3		
	 do 10 j=1,3		
	  do 5 k=1,3
	   do 1 l=1,3
	     dum=dum+t(i,k)*t(j,l)*a(k,l)
1	   continue
5	  continue
	 b(i,j)=dum
	 dum=0
10	 continue
15	continue

	return
	end
c**********************************************************************
	subroutine calcrot(ip,theta,phi,t)
c
c  This subroutine is taken from a paper by V. A. Schmidt
c  "On the Use of Orthogonal Transformations in the
c  Reduction of Paleomagnetic Data", J. Geomag. Geoelectr., 26,
c  475-486, 1974 and was writen by JDM 11/25/86
c  The design is for bedding, sample, or plunge
c  correction if variables are set as described below.
c 
c  Input: theta,phi (in degrees)
c
c               For Sample to In Situ Correction (ipar = 1)
c     theta   = sample azimuth
c     phi     = core dip
c 
c               For Bedding Tilt Correction (ipar = 2)
c     theta   = bedding dip direction - 90 (= strike right of dip)
c     phi     = dip of beds
c 
c               For Plunge Correction (ipar = 3)
c     theta   = plunge direction 
c     phi     = amount of plunge
c 
c Output: rotation matrix t
c

	implicit double precision (a-h,o-z)
	dimension t(3,3)
	pi=2.0*dasin(1.0d0)
	rad=180./pi
	
	do 1 i=1,3
	 do 1 j=1,3
1	t(i,j)=0.

	if(ip.eq.1) then	! sample to geographic
	  a = 360-theta
	  b = -(90-phi)
	  c = 0
	else if(ip.eq.2) then	! bedding tilt
	  a = -(theta)-90
	  b = -phi
	  c = theta + 90
	else if(ip.eq.3) then	! plunge correction
	  a = -theta
	  b = -phi
	  c = theta
	else
	  write(*,*) ' coordinate system not selected for calcrot '
	  return
	endif

	sa = dsin(a/rad)
	ca = dcos(a/rad)
	sb = dsin(b/rad)
	cb = dcos(b/rad)
	sc = dsin(c/rad)
	cc = dcos(c/rad)
	
	t(1,1) = ca*cb*cc-sa*sc
	t(1,2) = ca*cb*sc+sa*cc
	t(1,3) = -(ca*sb)
	t(2,1) = -(sa*cb*cc+ca*sc)
	t(2,2) = -(sa*cb*sc-ca*cc)
	t(2,3) = sa*sb
	t(3,1) = sb*cc
	t(3,2) = sb*sc
	t(3,3) = cb
	
	return
	end
c**********************************************************************
