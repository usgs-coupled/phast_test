SUBROUTINE XP_aplbce_thread(xp)
  ! ... Applies right hand side terms from b.c.
  USE machine_constants, ONLY: kdp
  USE mcb, only: ifacefbc, ifacelbc, fresur, &
    nfbc_cells, nlbc_cells, nrbc_cells, ndbc_cells, &
    flux_seg_m, flux_seg_first, flux_seg_last, leak_seg_first, leak_seg_last,  &
    river_seg_first, river_seg_last, &
    drain_seg_first, drain_seg_last, &
    albc, bblbc, blbc, denlbc, klbc, philbc, vislbc, zelbc, &
    arbc, bbrbc, brbc, denrbc, krbc, phirbc, visrbc, zerbc, &
    adbc, bbdbc, bdbc, kdbc, zedbc, visdbc, &
    mfsbc, qfflx, areafbc, mdbc
  USE mcg, only: nxy
  USE mcn, only: z
  USE mcp, only: fdtmth, den0, gz
  USE mcv, only: frac, p
  USE phys_const
  USE XP_module, ONLY: Transporter
  IMPLICIT NONE
  TYPE (Transporter) :: xp
  REAL(KIND=kdp) :: qfbc, qn, ufdt2, ufrac, uphim, uzav
  REAL(KIND=kdp) :: hrbc
  INTEGER :: imod, k, ks, l1, lc, ls, m, mc, ms  
  LOGICAL :: erflg  
  REAL(KIND=kdp) :: qsbc, qsbc2
  !     ------------------------------------------------------------------
  !...
  ufdt2 = fdtmth  
  ! ... Specified P or C b.c. terms are applied in APLBCI
  ! ... Apply specified flux b.c. dispersive and advective terms
  erflg = .FALSE.
  ! ... Specified flux b.c.
  DO lc=1,nfbc_cells
     m = flux_seg_m(lc)
     IF(m == 0) CYCLE     ! ... dry column
     DO ls=flux_seg_first(lc),flux_seg_last(lc)
        ufrac = 1._kdp
        IF(ABS(ifacefbc(ls)) < 3) ufrac = frac(m)
        IF(fresur .AND. ifacefbc(ls) == 3 .AND. frac(m) <= 0._kdp) THEN
           ! ... Redirect the flux from above to the free-surface cell
           l1 = MOD(m,nxy)
           IF(l1 == 0) l1 = nxy
           m = mfsbc(l1)
        ENDIF
        IF (m <= 0) EXIT          ! ... dry column; skip to next flux bc column
        qn = qfflx(ls)*areafbc(ls)
        IF(qn <= 0.) THEN             ! ... Outflow
           qfbc = den0*qn*ufrac
           qsbc = qfbc*xp%c_w(m)
        ELSE                          ! ... Inflow
           qfbc = den0*qn*ufrac
           qsbc = qfbc*xp%cfbc(ls)
        ENDIF
!$$        rf(m) = rf(m) + ufdt2*qfbc
        qsbc2 = xp%qsflx(ls)*areafbc(ls)*ufrac
        xp%rs(m) = xp%rs(m) + ufdt2*(qsbc2 + qsbc)
     END DO
  END DO
  ! ... Calculate leakage b.c. coefficients
  ! ...      only for horizontal coordinates
  DO lc=1,nlbc_cells
     m = xp%leak_seg_m(lc)
     IF(m == 0) CYCLE     ! ... dry column
     DO ls=leak_seg_first(lc),leak_seg_last(lc)
        albc(ls) = 0._kdp
        blbc(ls) = 0._kdp
        ufrac = frac(m)  
        ! ... No flow to or from empty cells, and attenuate flows
        ! ...      at partially saturated cells.
        imod = MOD(m,nxy)
        k = (m - imod)/nxy + MIN(1,imod)
        IF(ABS(ifacelbc(ls)) == 3) THEN  
           uzav = zelbc(ls) - .5_kdp*bblbc(ls)
        ELSE
           uzav = zelbc(ls)
        ENDIF
        uphim = p(m) + gz*(denlbc(ls) - den0)*uzav
        blbc(ls) = klbc(ls)/vislbc(ls)
        albc(ls) = blbc(ls)*((denlbc(ls)*philbc(ls) - den0*gz*z(k)) - uphim)
        ! ... Attenuate the flow rate for a partially saturated cell with
        ! ...      leakage through a lateral face
        IF(ABS(ifacelbc(ls)) < 3) THEN  
           albc(ls) = ufrac*albc(ls)
           blbc(ls) = ufrac*blbc(ls)
        ENDIF
     END DO
  END DO
  ! ... Calculate river leakage b.c. coefficients
  ! ...      only for horizontal coordinates; No lateral river leakage
  DO lc=1,nrbc_cells
     mc = xp%river_seg_m(lc)
     IF(mc == 0) CYCLE     ! ... dry column
     DO ls=river_seg_first(lc),river_seg_last(lc)
        arbc(ls) = 0._kdp
        brbc(ls) = 0._kdp
        uzav = zerbc(ls) - .5_kdp*bbrbc(ls)     
        ms = xp%mrbc(ls)        ! ... current river segment cell for aquifer head
        ! ... now ms = mc
        imod = MOD(ms,nxy)
        ks = (ms - imod)/nxy + MIN(1,imod)
        uphim = p(ms) + gz*(denrbc(ls) - den0)*uzav
        brbc(ls) = krbc(ls)/visrbc(ls)
        hrbc = phirbc(ls)/gz
        if(hrbc > zerbc(ls)) then      ! ... treat as river
            arbc(ls) = brbc(ls)*((denrbc(ls)*phirbc(ls) - den0*gz*z(ks)) - uphim)
        else                           ! ... treat as drain 
            arbc(ls) = brbc(ls)*(denrbc(ls)*gz*zerbc(ls) - (p(ms) + den0*gz*z(ks)))
            if (arbc(ls) .gt. 0.0_kdp) then
                arbc(ls) = 1.0e51_kdp
            endif
        end if
     END DO
  END DO
!!$  DO ls=1,nrbc_seg
!!$     m = mrbc(ls)     ! ... current f.s. cell
!!$     arbc(ls) = 0._kdp
!!$     brbc(ls) = 0._kdp
!!$     imod = mod(m,nxy)
!!$     k = (m - imod)/nxy + min(1,imod)
!!$     uzav = zerbc(ls) - .5_kdp*bbrbc(ls)
!!$     uphim = p(m) + gz*(denrbc(ls) - den0)*uzav
!!$     brbc(ls) = krbc(ls)/visrbc(ls)
!!$     arbc(ls) = brbc(ls)*((denrbc(ls)*phirbc(ls) - den0*gz*z(k)) - uphim)
!!$     ! ... No lateral river leakage
!!$  END DO
!!$  DO lc=1,nrbc_cells
!!$     m = river_seg_index(lc)%m
!!$     IF(m == 0) CYCLE
!!$     ! ... Calculate current net aquifer leakage flow rate
!!$     ! ...      Possible attenuation included explicitly
!!$     qm_net = 0._kdp
!!$     DO ls=river_seg_index(lc)%seg_first,river_seg_index(lc)%seg_last
!!$        qn = arbc(ls)
!!$        IF(qn < 0._kdp) THEN
!!$           ! ... Outflow
!!$           qm_net = qm_net + den0*qn
!!$        ELSE  
!!$           ! ... Inflow
!!$           ! ... Limit the flow rate for a river leakage
!!$           qlim = brbc(ls)*(denrbc(ls)*phirbc(ls) - gz*(denrbc(ls)*(zerbc(ls)-0.5_kdp*bbrbc(ls))  &
!!$                - den0*bbrbc(ls)))
!!$           qn = MIN(qn,qlim)
!!$           qm_net = qm_net + denrbc(ls)*qn
!!$        ENDIF
!!$     END DO
!!$     qfrbc(lc) = qm_net
!!$     IF(qm_net < 0._kdp) THEN  
!!$        ! ... Outflow
!!$        DO  is=1,ns
!!$           qsrbc(lc,is) = qfrbc(lc)*c(m,is)
!!$        END DO
!!$     ELSE
!!$        ! ... Inflow
!!$        qm_in = 0._kdp
!!$        sum_cqm_in = 0._kdp
!!$        DO ls=river_seg_index(lc)%seg_first,river_seg_index(lc)%seg_last
!!$           qn = arbc(ls)
!!$           IF(qn > 0._kdp) THEN  
!!$              ! ... Inflow
!!$              ! ... Limit the flow rate for a river leakage
!!$              qlim = brbc(ls)*(denrbc(ls)*phirbc(ls) - gz*(denrbc(ls)*  &
!!$                   (zerbc(ls)-0.5_kdp*bbrbc(ls)) - den0*bbrbc(ls)))
!!$              qn = MIN(qn,qlim)
!!$              qm_in = qm_in + denrbc(ls)*qn
!!$              sum_cqm_in = sum_cqm_in + denrbc(ls)*qn*crbc(ls,is)  
!!$           ENDIF
!!$        END DO
!!$        cavg = sum_cqm_in/qm_in
!!$        DO  is=1,ns
!!$           qsrbc(lc,is) = qfrbc(lc)*cavg
!!$        END DO
!!$     END IF
!!$  END DO
  ! ... Calculate drain leakage b.c. coefficients
  ! ...      only for horizontal coordinates; No lateral drain leakage
  DO lc=1,ndbc_cells
     mc = xp%drain_seg_m(lc)
     IF(mc == 0) CYCLE     ! ... dry column; skip to next drain bc column
     DO ls=drain_seg_first(lc),drain_seg_last(lc)
        adbc(ls) = 0._kdp
        bdbc(ls) = 0._kdp
        ms = mdbc(ls)        ! ... current drain segment cell for aquifer head
        ! ... now ms = mc
        imod = MOD(ms,nxy)
        ks = (ms - imod)/nxy + MIN(1,imod)
        bdbc(ls) = kdbc(ls)/visdbc
        adbc(ls) = bdbc(ls)*(den0*gz*zedbc(ls) - (p(ms) + den0*gz*z(ks)))
        if (adbc(ls) .gt. 0.0_kdp) then
            adbc(ls) = 1.0e51_kdp
        endif  
     END DO
  END DO
END SUBROUTINE XP_aplbce_thread
SUBROUTINE XP_aplbce(xp)
  ! ... Applies right hand side terms from b.c.
  USE machine_constants, ONLY: kdp
  USE mcb
  USE mcg, only: nxy
  USE mcn, only: z
  USE mcp, only: fdtmth, den0, gz
  USE mcv, only: frac, p
  USE phys_const
  USE XP_module, only: Transporter
  IMPLICIT NONE
  TYPE (Transporter) :: xp
  REAL(KIND=kdp) :: qfbc, qn, ufdt2, ufrac, uphim, uzav
  REAL(KIND=kdp) :: hrbc
  INTEGER :: imod, k, ks, l1, lc, ls, m, mc, ms  
  LOGICAL :: erflg  
  REAL(KIND=kdp) :: qsbc, qsbc2
  !     ------------------------------------------------------------------
  !...
  ufdt2 = fdtmth  
  ! ... Specified P or C b.c. terms are applied in APLBCI
  ! ... Apply specified flux b.c. dispersive and advective terms
  erflg = .FALSE.
  ! ... Specified flux b.c.
  DO lc=1,nfbc_cells
     m = flux_seg_m(lc)
     IF(m == 0) CYCLE     ! ... dry column
     DO ls=flux_seg_first(lc),flux_seg_last(lc)
        ufrac = 1._kdp
        IF(ABS(ifacefbc(ls)) < 3) ufrac = frac(m)
        IF(fresur .AND. ifacefbc(ls) == 3 .AND. frac(m) <= 0._kdp) THEN
           ! ... Redirect the flux from above to the free-surface cell
           l1 = MOD(m,nxy)
           IF(l1 == 0) l1 = nxy
           m = mfsbc(l1)
        ENDIF
        IF (m <= 0) EXIT          ! ... dry column; skip to next flux bc column
        qn = qfflx(ls)*areafbc(ls)
        IF(qn <= 0.) THEN             ! ... Outflow
           qfbc = den0*qn*ufrac
           qsbc = qfbc*xp%c_w(m)
        ELSE                          ! ... Inflow
           qfbc = den0*qn*ufrac
           qsbc = qfbc*xp%cfbc(ls)
        ENDIF
!$$        rf(m) = rf(m) + ufdt2*qfbc
        qsbc2 = xp%qsflx(ls)*areafbc(ls)*ufrac
        xp%rs(m) = xp%rs(m) + ufdt2*(qsbc2 + qsbc)
     END DO
  END DO
  ! ... Calculate leakage b.c. coefficients
  ! ...      only for horizontal coordinates
  DO lc=1,nlbc_cells
     m = leak_seg_m(lc)
     IF(m == 0) CYCLE     ! ... dry column
     DO ls=leak_seg_first(lc),leak_seg_last(lc)
        albc(ls) = 0._kdp
        blbc(ls) = 0._kdp
        ufrac = frac(m)  
        ! ... No flow to or from empty cells, and attenuate flows
        ! ...      at partially saturated cells.
        imod = MOD(m,nxy)
        k = (m - imod)/nxy + MIN(1,imod)
        IF(ABS(ifacelbc(ls)) == 3) THEN  
           uzav = zelbc(ls) - .5_kdp*bblbc(ls)
        ELSE
           uzav = zelbc(ls)
        ENDIF
        uphim = p(m) + gz*(denlbc(ls) - den0)*uzav
        blbc(ls) = klbc(ls)/vislbc(ls)
        albc(ls) = blbc(ls)*((denlbc(ls)*philbc(ls) - den0*gz*z(k)) - uphim)
        ! ... Attenuate the flow rate for a partially saturated cell with
        ! ...      leakage through a lateral face
        IF(ABS(ifacelbc(ls)) < 3) THEN  
           albc(ls) = ufrac*albc(ls)
           blbc(ls) = ufrac*blbc(ls)
        ENDIF
     END DO
  END DO
  ! ... Calculate river leakage b.c. coefficients
  ! ...      only for horizontal coordinates; No lateral river leakage
  DO lc=1,nrbc_cells
     mc = river_seg_m(lc)
     IF(mc == 0) CYCLE     ! ... dry column
     DO ls=river_seg_first(lc),river_seg_last(lc)
        arbc(ls) = 0._kdp
        brbc(ls) = 0._kdp
        uzav = zerbc(ls) - .5_kdp*bbrbc(ls)     
        ms = mrbc(ls)        ! ... current river segment cell for aquifer head
        ! ... now ms = mc
        imod = MOD(ms,nxy)
        ks = (ms - imod)/nxy + MIN(1,imod)
        uphim = p(ms) + gz*(denrbc(ls) - den0)*uzav
        brbc(ls) = krbc(ls)/visrbc(ls)
        hrbc = phirbc(ls)/gz
        if(hrbc > zerbc(ls)) then      ! ... treat as river
            arbc(ls) = brbc(ls)*((denrbc(ls)*phirbc(ls) - den0*gz*z(ks)) - uphim)
        else                           ! ... treat as drain 
            arbc(ls) = brbc(ls)*(denrbc(ls)*gz*zerbc(ls) - (p(ms) + den0*gz*z(ks)))
            if (arbc(ls) .gt. 0.0_kdp) then
                arbc(ls) = 1.0e51_kdp
            endif  
        endif
     END DO
  END DO
!!$  DO ls=1,nrbc_seg
!!$     m = mrbc(ls)     ! ... current f.s. cell
!!$     arbc(ls) = 0._kdp
!!$     brbc(ls) = 0._kdp
!!$     imod = mod(m,nxy)
!!$     k = (m - imod)/nxy + min(1,imod)
!!$     uzav = zerbc(ls) - .5_kdp*bbrbc(ls)
!!$     uphim = p(m) + gz*(denrbc(ls) - den0)*uzav
!!$     brbc(ls) = krbc(ls)/visrbc(ls)
!!$     arbc(ls) = brbc(ls)*((denrbc(ls)*phirbc(ls) - den0*gz*z(k)) - uphim)
!!$     ! ... No lateral river leakage
!!$  END DO
!!$  DO lc=1,nrbc_cells
!!$     m = river_seg_index(lc)%m
!!$     IF(m == 0) CYCLE
!!$     ! ... Calculate current net aquifer leakage flow rate
!!$     ! ...      Possible attenuation included explicitly
!!$     qm_net = 0._kdp
!!$     DO ls=river_seg_index(lc)%seg_first,river_seg_index(lc)%seg_last
!!$        qn = arbc(ls)
!!$        IF(qn < 0._kdp) THEN
!!$           ! ... Outflow
!!$           qm_net = qm_net + den0*qn
!!$        ELSE  
!!$           ! ... Inflow
!!$           ! ... Limit the flow rate for a river leakage
!!$           qlim = brbc(ls)*(denrbc(ls)*phirbc(ls) - gz*(denrbc(ls)*(zerbc(ls)-0.5_kdp*bbrbc(ls))  &
!!$                - den0*bbrbc(ls)))
!!$           qn = MIN(qn,qlim)
!!$           qm_net = qm_net + denrbc(ls)*qn
!!$        ENDIF
!!$     END DO
!!$     qfrbc(lc) = qm_net
!!$     IF(qm_net < 0._kdp) THEN  
!!$        ! ... Outflow
!!$        DO  is=1,ns
!!$           qsrbc(lc,is) = qfrbc(lc)*c(m,is)
!!$        END DO
!!$     ELSE
!!$        ! ... Inflow
!!$        qm_in = 0._kdp
!!$        sum_cqm_in = 0._kdp
!!$        DO ls=river_seg_index(lc)%seg_first,river_seg_index(lc)%seg_last
!!$           qn = arbc(ls)
!!$           IF(qn > 0._kdp) THEN  
!!$              ! ... Inflow
!!$              ! ... Limit the flow rate for a river leakage
!!$              qlim = brbc(ls)*(denrbc(ls)*phirbc(ls) - gz*(denrbc(ls)*  &
!!$                   (zerbc(ls)-0.5_kdp*bbrbc(ls)) - den0*bbrbc(ls)))
!!$              qn = MIN(qn,qlim)
!!$              qm_in = qm_in + denrbc(ls)*qn
!!$              sum_cqm_in = sum_cqm_in + denrbc(ls)*qn*crbc(ls,is)  
!!$           ENDIF
!!$        END DO
!!$        cavg = sum_cqm_in/qm_in
!!$        DO  is=1,ns
!!$           qsrbc(lc,is) = qfrbc(lc)*cavg
!!$        END DO
!!$     END IF
!!$  END DO
  ! ... Calculate drain leakage b.c. coefficients
  ! ...      only for horizontal coordinates; No lateral drain leakage
  DO lc=1,ndbc_cells
     mc = drain_seg_m(lc)
     IF(mc == 0) CYCLE     ! ... dry column; skip to next drain bc column
     DO ls=drain_seg_first(lc),drain_seg_last(lc)
        adbc(ls) = 0._kdp
        bdbc(ls) = 0._kdp
        ms = mdbc(ls)        ! ... current drain segment cell for aquifer head
        ! ... now ms = mc
        imod = MOD(ms,nxy)
        ks = (ms - imod)/nxy + MIN(1,imod)
        bdbc(ls) = kdbc(ls)/visdbc
        adbc(ls) = bdbc(ls)*(den0*gz*zedbc(ls) - (p(ms) + den0*gz*z(ks)))
        if (adbc(ls) .gt. 0.0_kdp) then
            adbc(ls) = 1.0e51_kdp
        endif     
     END DO
  END DO
END SUBROUTINE XP_aplbce
