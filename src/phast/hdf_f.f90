!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! TODO
!
! Preconditions:
!   Must be called before first call to EQUILIBRATE
!   
!   Valid mcg{NX, NY, NZ}         => READ1
!   Valid mcn{X, Y, Z}            => READ2
!   Valid mcw{NKSWEL, NWEL, MWEL} => READ2
!   Valid mcch{UTULBL}            => INIT1
!   Valid mcb{IBC}                => INIT2
!   Valid mcb{MSBC, nsbc}         => INIT2
!   Valid mcb{MFBC, nfbc}         => INIT2
!   Valid mcb{MLBC, nlbc}         => INIT2
!   Valid mcb{MRBC, nrbc}         => INIT2
!
! Postconditions:
!   TODO
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
SUBROUTINE hdf_write_invariant(mpi_myself)
  USE mcg, ONLY: nx, ny, nz
  USE mcn, ONLY: x, y, z
  USE mcw, ONLY: nkswel, nwel, mwel
  USE mcb, ONLY: ibc, msbc, nsbc, mfbc, nfbc, mlbc, nlbc, mrbc, nrbc_seg,  &
       mdbc, ndbc_seg, flux_seg_index, leak_seg_index, nfbc_cells, nlbc_cells   ! ... b.c. information
  USE mcch, ONLY: utulbl
  IMPLICIT NONE
  INTEGER, INTENT(IN) :: mpi_myself
!!$  INTEGER :: i, j, k, m
  INTEGER :: a_err, i
  INTEGER :: iwel
  INTEGER :: index, nwbc                      ! Well index and node count
  INTEGER, DIMENSION(:), ALLOCATABLE  :: MWBC ! Well nodes
  ! ... Set string for use with RCS ident command
  INTEGER, DIMENSION(nrbc_seg) :: temp_rbc
  INTEGER, DIMENSION(ndbc_seg) :: temp_dbc
  INTEGER, DIMENSION(nfbc_cells) :: temp_fbc
  INTEGER, DIMENSION(nlbc_cells) :: temp_lbc
  CHARACTER(LEN=80) :: ident_string='$Id$'
  !     ------------------------------------------------------------------
  !...
  IF (mpi_myself == 0) THEN
     CALL HDF_INIT_INVARIANT();
  ENDIF
  
  CALL HDF_WRITE_GRID(X, Y, Z, NX, NY, NZ, IBC, UTULBL)
  
  !
  ! Count Well nodes
  !
  IF (mpi_myself == 0) THEN
  nwbc = 0
  DO IWEL = 1, NWEL
     nwbc = nwbc + NKSWEL(IWEL)
  END DO
  
  !
  ! Store Well Nodes
  !
     ALLOCATE (MWBC(nwbc), stat = a_err)
     IF (a_err.NE.0) THEN  
        PRINT *, 'HDF Error: Unable to allocate MWBC'
        STOP  
     ENDIF
     
     index = 1;
     DO 50 IWEL = 1, NWEL
        DO 40 i=1,NKSWEL(IWEL)
           MWBC(index) = MWEL(IWEL, i)
           index = index + 1
40      END DO
50   END DO
     if(nwbc > 0) CALL HDF_WRITE_FEATURE('Wells'    , MWBC, nwbc)
     if(nsbc > 0) CALL HDF_WRITE_FEATURE('Specified', MSBC, nsbc)
     !if(nfbc > 0) CALL HDF_WRITE_FEATURE('Flux'     , MFBC, nfbc)
     if(nfbc_cells > 0) then
        do i = 1, nfbc_cells
           temp_fbc(i) = flux_seg_index(i)%m
        enddo
        CALL HDF_WRITE_FEATURE('Flux'    , temp_fbc, nfbc_cells)
     endif
     !if(nlbc > 0) CALL HDF_WRITE_FEATURE('Leaky'    , MLBC, nlbc)
     if(nlbc_cells > 0) then
        do i = 1, nlbc_cells
           temp_lbc(i) = leak_seg_index(i)%m
        enddo
        CALL HDF_WRITE_FEATURE('Leaky'    , temp_lbc, nlbc_cells)
     endif
     if(nrbc_seg > 0) then
        do i = 1, nrbc_seg
           temp_rbc(i) = mrbc(i)
        enddo
        CALL HDF_WRITE_FEATURE('River', temp_rbc, nrbc_seg)
     endif
     if(ndbc_seg > 0) then
        do i = 1, ndbc_seg
           temp_dbc(i) = mdbc(i)
        enddo
        CALL HDF_WRITE_FEATURE('Drain', temp_dbc, ndbc_seg)
     end if
  ! 
  ! Free Well Nodes
  !
     DEALLOCATE (MWBC, stat = a_err)
     IF (a_err /= 0) THEN  
        PRINT *, 'HDF Error: Unable to deallocate MWBC'  
        STOP  
     ENDIF
  ENDIF
  IF (mpi_myself == 0) THEN
     CALL HDF_FINALIZE_INVARIANT()
  ENDIF
  
END SUBROUTINE hdf_write_invariant

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! TODO
!
! Preconditions:
!   TODO
! Postconditions:
!   TODO
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
SUBROUTINE hdf_begin_time_step
  USE mcc, ONLY: PRHDFHI, PRHDFCI, prhdfvi, solute
  USE mcv, ONLY: TIME
  USE mcp, ONLY: CNVTMI
  USE hdf_media, ONLY: pr_hdf_media
  IMPLICIT NONE
  INTEGER time_step_fscalar_count
!*****seems like this could be simplified with direct pass of integer flag***

  time_step_fscalar_count = 0
  if (prhdfhi == 1) then
     time_step_fscalar_count = time_step_fscalar_count + 1
  endif
  if (pr_hdf_media) then
    time_step_fscalar_count = time_step_fscalar_count + 5
    if (solute) then
        time_step_fscalar_count = time_step_fscalar_count + 4
    endif
  endif
  
        

  CALL HDF_OPEN_TIME_STEP(TIME, CNVTMI, prhdfci, prhdfvi, time_step_fscalar_count)
END SUBROUTINE hdf_begin_time_step

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! TODO
!
! Preconditions:
!   TODO
! Postconditions:
!   TODO
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
SUBROUTINE hdf_end_time_step()
!!$  USE machine_constants, ONLY: kdp
  USE mcv,               ONLY: frac, vx_node, vy_node, vz_node
  USE mcp,               ONLY: cnvli, cnvvli
  USE mcc,               ONLY: prhdfhi, prhdfvi, vmask, ntprhdfv, ntprhdfh
  USE mcch,              ONLY: unitl
  USE mg2,               ONLY: hdprnt
  USE hdf_media,         ONLY: pr_hdf_media
  IMPLICIT NONE
  IF (prhdfhi == 1) THEN
     !!NOTE: Don't need IBC array since hdf_write_invariant accounts for inactive cells
     CALL prntar_hdf(hdprnt, frac, cnvli, 'Fluid Head('//unitl//')')
     ntprhdfh = ntprhdfh+1
  END IF
  
  if (pr_hdf_media) then
    CALL media_hdf
    pr_hdf_media = .false.
  endif

  IF (prhdfvi == 1) THEN
     ! convert velocity to user units, should be last use of these 
     ! velocities for time step
     vx_node = vx_node*cnvvli
     vy_node = vy_node*cnvvli
     vz_node = vz_node*cnvvli
     CALL hdf_vel(vx_node, vy_node, vz_node, vmask)
     ntprhdfv = ntprhdfv+1
  END IF
  CALL HDF_CLOSE_TIME_STEP()  
END SUBROUTINE hdf_end_time_step

