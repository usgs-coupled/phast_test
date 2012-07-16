SUBROUTINE calcc(c, dc, denn, dp, dpkm, dpkp, dt, fracn, fracnzkp,  &
     ibckm, ibckp, jeq, k, &
     p, pkm, pkp, pmchv, pmcv, pmhv, pv, pvk, t, z, zkm, zkp, deltim)
  ! ... Calculates the C-matrix coefficients in Gauss elimination
  ! ...      form (upper triangular)
  ! ...      unless SVBC is true or FRESUR, then no Gauss elimination
  ! ...      JEQ -  equation index: 1 - solute, 2 - heat, 3 - flow
  ! ... This routine is called once for each cell
  ! ... DENT,DENC are zero if no heat or solute equation respectively
  USE machine_constants, ONLY: kdp
  USE mcb
!$$  USE mcb_w
  USE mcc
  USE mcg
  USE mcm
!$$  USE mcm_w
  USE mcp, ONLY: declam, denc, denp, dent, fdtmth, gz
  IMPLICIT NONE
  REAL(KIND=kdp), INTENT(IN) :: c, dc, denn, dp, dpkm, dpkp, dt, fracn, fracnzkp
  INTEGER, INTENT(IN) :: ibckm, ibckp, jeq, k  
  REAL(KIND=kdp), INTENT(IN) :: p, pkm, &
       pkp, pmchv, pmcv, pmhv, pv, pvk, t, z, zkm, zkp, deltim
  ! ...
  REAL(KIND=kdp) :: cnp, dennp, dfdpk, dfdpkm, ufracnp, m21, &
       m31, m32, pk, pkmnp, pknp, pmchdt, pmcvdt, pmhvdt, pnp, pvdtn, &
       pvkdtn, tnp, ufd, ufd2, upct, upv, zfsnp, zk
  REAL(KIND=kdp), PARAMETER :: epssat = 1.e-6_kdp  
  ! ... Set string for use with RCS ident command
  CHARACTER(LEN=80) :: ident_string='$Id$'
  !     ------------------------------------------------------------------
  !...
  pnp = p+dp
  pkmnp = pkm+dpkm
  cnp = c+dc
  dennp = denn+denp*dp+dent*dt+denc*dc
  pvdtn = pv/deltim
  pvkdtn = pvk/deltim
  pmcvdt = pmcv/deltim
  upv = pvdtn+pvkdtn
  declam  =  0._kdp
  ufd = fdtmth*declam
  ufd2 = (1._kdp-fdtmth)*declam
  cfp = 0._kdp
  efp = 0._kdp
  csp = 0._kdp
  esp = 0._kdp
  IF(fresur .AND. (k == nz .OR. fracn < 1._kdp .OR.  &
       (fracn == 1._kdp .AND. fracnzkp == 0._kdp))) THEN
     ! ... Case of free-surface cell
     ! ... Calculate partial derivatives of saturated fraction with respect
     ! ...      to pk, pk-1
     ! ... Calculate estimated saturated fraction at n+1
     zk = z
     pk = p
     pknp = pnp
     IF(k == 1) THEN
        ! ... Bottom plane; hydrostatic
        IF(pk > 0._kdp) THEN
           dfdpkm = 0._kdp
           dfdpk = 2./((zkp-zk)*(denn*gz))
           zfsnp = pknp/(dennp*gz)+zk      ! Hydrostatic
           ufracnp = 2.*(zfsnp-zk)/(zkp-zk)
        ELSE
           ! ... Don't let cell go totally dry
           ufracnp = epssat
        END IF
     ELSE IF(k == nz) THEN
        ! ... Top plane
        dfdpkm = 0._kdp
        dfdpk = 2./((zk-zkm)*(denn*gz))
        zfsnp = pknp/(dennp*gz)+zk      ! Hydrostatic
        ufracnp = (2.*zfsnp-(zk+zkm))/(zk-zkm)
        ! ... Don't let cell go totally dry
        ufracnp = MAX(epssat,ufracnp)
     ELSE
        !.... Intermediate plane
        IF(ibckm == -1) THEN
           ! ... Treat as bottom plane
           IF(pk > 0._kdp) THEN
              dfdpkm = 0._kdp
              dfdpk = 2./((zkp-zk)*(denn*gz))
              zfsnp = pknp/(dennp*gz)+zk      ! Hydrostatic
              ufracnp = 2.*(zfsnp-zk)/(zkp-zk)
           ELSE
              ! ... Don't let cell go totally dry
              ufracnp = epssat
           END IF
        ELSEIF(ibckp == -1) THEN
           ! ... Treat as top plane
           IF(pk > 0._kdp) THEN
              dfdpkm = 0._kdp
              dfdpk = 2./((zk-zkm)*(denn*gz))
              zfsnp = pknp/(dennp*gz)+zk      ! Hydrostatic
              ufracnp = (2.*zfsnp-(zk+zkm))/(zk-zkm)
           else
              dfdpkm = 0._kdp
              dfdpk = 2./((zk-zkm)*(denn*gz))
              zfsnp = pknp/(dennp*gz)+zk      ! Hydrostatic
              ufracnp = (2.*zfsnp-(zk+zkm))/(zk-zkm)
              ! ... Don't let cell go totally dry
              ufracnp = MAX(epssat,ufracnp)
           end if
        ELSE
           ! ... True intermediate plane
           dfdpkm = 0._kdp
           dfdpk = 2./((zkp-zkm)*(denn*gz))
           zfsnp = pknp/(dennp*gz)+zk      ! Hydrostatic
           ufracnp = (2.*zfsnp-(zk+zkm))/(zkp-zkm)
           ! ... Don't let cell go totally dry
           ufracnp = MAX(epssat,ufracnp)
        END IF
     END IF
     IF(solute) THEN
        IF(jeq == 3) THEN
           c11 = (dennp*ufracnp+c*fracn*denc)*upv+ ufd2*(pv+pvk)*ufracnp*(denn+denc*c)
           c12 = 0._kdp
           c13 = dennp*c*dfdpk*upv
           csp = dennp*c*dfdpkm*upv
           RETURN
        ELSE            ! ... jeq = 1
           c11 = (dennp*ufracnp+c*fracn*denc)*upv+ ufd2*(pv+pvk)*fracn*(denn+denc*c)
           c12 = 0._kdp
           c13 = dennp*c*dfdpk*upv+ ufd2*(pv+pvk)*denn*c*dfdpk
           csp = dennp*c*dfdpkm*upv+ ufd2*(pv+pvk)*denn*c*dfdpkm
        END IF
     END IF
     c31 = fracn*denc*pvdtn
     c32 = 0._kdp
     c33 = dennp*dfdpk*pvdtn
     c34 = 0._kdp
     c35 = 0._kdp
     cfp = dennp*dfdpkm*pvdtn
  ELSE
     ! ... Case of a saturated cell
     IF(solute) THEN
        c11 = (denc*c+dennp)*upv
        c12 = dent*c*upv
        c13 = denp*c*upv+dennp*cnp*pmcvdt
        ! ... Apply decay
! *** not necessary, done in reaction step
!!$        c11 = c11+ufd*(denc*cnp+denn)*(pv+pvk)
!!$        c12 = c12+ufd*dent*cnp*(pv+pvk)
!!$        c13 = c13+ufd*(denp*(pv+pvk)+dennp*pmcv)*cnp
        IF(jeq == 3) RETURN
     END IF
     c31 = denc*pvdtn
     c32 = 0._kdp
     c33 = denp*pvdtn+dennp*pmcvdt
     c34 = 0._kdp
     c35 = 0._kdp
  END IF
  IF(svbc .OR. .NOT.gausel) RETURN
  ! ... Gauss elimination
  !...***not applicable for constant density
  IF(jeq == 1) THEN
        m31 = c31/c11
        c32 = c32-m31*c12
        c33 = c33-m31*c13
        c34 = -m31
  END IF
END SUBROUTINE calcc
