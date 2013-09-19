SUBROUTINE mtoijk(m,i,j,k,nx,ny)
  ! ... Returns the index (I,J,K) of the point with natural index M.
  !
  IMPLICIT NONE
  INTEGER, INTENT(IN) :: m
  INTEGER, INTENT(OUT) :: i, j, k
  INTEGER, INTENT(IN) :: nx, ny
  !
  INTEGER :: imod, kr, nxy
  ! ... Set string for use with RCS ident command
  CHARACTER(LEN=80) :: ident_string='$Id: mtoijk.f90 3780 2009-11-18 21:28:13Z dlpark $'
  !     ------------------------------------------------------------------
  !...
  nxy = nx*ny
  imod = MOD(m,nxy)
  k = (m-imod)/nxy + MIN(1,imod)
  kr = imod
  IF (kr == 0) kr = nxy
  imod = MOD(kr,nx)
  j = (kr-imod)/nx + MIN(1,imod)
  i = imod
  IF (i == 0) i = nx
END SUBROUTINE mtoijk
