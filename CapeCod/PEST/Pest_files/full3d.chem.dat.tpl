KNOBS
	-step 	10
	-pe	10
SOLUTION_MASTER_SPECIES
[N2] 	[N2]	0.0	28	28
SOLUTION_SPECIES
[N2] = [N2]
	log_k 0

PHASES
Strengite           146
        FePO4:2H2O = Fe+3 + PO4-3 + 2H2O 
        log_k           -26.4
        delta_h -2.030 kcal
SOLUTION_MASTER_SPECIES
	Amm		AmmH+	0.0	AmmH		18.0
	Toc	Toc	0.0	Toc	12.0
	Soc	Soc	0.0	Soc	12.0

SOLUTION_SPECIES

	H2O + 0.01e- = H2O-0.01
	log_k	-9


	Toc = Toc
	log_k 0.0

	Soc = Soc
	log_k 0.0

AmmH+ = AmmH+
	log_k	0.0
	-gamma	2.5	0.0

AmmH+ = Amm + H+
	log_k	-9.252
	delta_h 12.48	kcal

AmmH+ + SO4-2 = AmmHSO4-
	log_k	1.11


SURFACE_MASTER_SPECIES
	Cation	CationOH
	Anion 	AnionOH

SURFACE_SPECIES

#Anion
	AnionOH = AnionOH
	log_k	0.0

        AnionOH + H+ = AnionOH2+
	log_k	%    AnionOH2_k %

        AnionOH = AnionO- + H+
        log_k 		-7.0

 	AnionOH + PO4-3 + 2H+ = AnionHPO4- + H2O
	log_k	 	 %   AnionHPO4_k %

#Cation
	CationOH = CationOH
	log_k	0.0

        CationOH + H+ = CationOH2+
	log_k	%   CationOH2_k % 

        CationOH = CationO- + H+
        log_k 		 -7.

        CationOH + Ca+2 = CationOCa+ + H+
        log_k 		 %   Cation_k    % 

        CationOH + Mg+2 = CationOMg+ + H+
        log_k 		 %   Cation_k    % 

        CationOH + K+ = CationOK + H+
        log_k 		 %   Cation_k    % 

        CationOH + Na+ = CationONa + H+
        log_k 		 %   Cation_k    % 

 	CationOH + AmmH+ = CationOAmmH + H+
        log_k 		 %   Cation_k    % 

        CationOH + Mn+2 = CationOMn+ + H+
        log_k 		 %   Cation_k    % 

        CationOH + Fe+2 = CationOFe+ + H+
        log_k 		 %   Cation_k    % 

TITLE 1-D evolution of sewage plume
#    Uncontaminated groundwater based on Doug Kent's criteria from memo of 4/98
SOLUTION 1 Uncontaminated groundwater
	pH	5.6
	pe	7.0
	temp	14.0
	units	umol/L
	Ca	29
	Mg      31
	Na	200
	K	10
	Fe	0.0
	Mn	0.64
#	Al	0.0
#	Si	102
	Cl	140     charge
	S(6)	86
	C(4)    28
	N(5)	0.0
#	N(3)	0.0
#	N(organic) 0.0
#	P	2.3
#	F	0.0
	O(0)	%        DO     % 
#	Toc	26
#	B	1.0
#	[N2]    100
#    Sewage effluent data from Table 1 in notes. Covers 1974-78, 1979,
#    1980, 1983, 1988, 1994. Values are averaged.
SOLUTION 2 Sewage effluent with oxygen, double P
	pH	6.00
	pe	7.0
	temp	14.0
	units	umol/L
	Ca	335
	Mg	170
	Na     	2100
	K	240
	Fe	4.5
	Mn	0.4
#	Al	1.9
#	Si	210
	Cl	990    charge
	S(6)	290
	C(4)   	1200
#	Amm	180
	Amm	 1.8000000000e+02
	N(5)	1050     # based on 12 mg/L N (Smith)
#	N(3)	73
#	N(organic)  230
	P	%   Effluent_P %
#	F	33
	O(0)	%            DO % 
	Toc	%           TOC % # 1.6000000000e+03
#	Toc	1667	# based on 20 mg/L average
	Soc	%           SOC % # 1000
#	B	56
#	[N2]    100
SOLUTION 3 Rain
	units umol/L
	pH	5
	N(5)	1	charge
	O(0)	1	O2(g) -0.7
	[N2]    100
#	Mn	0.4
SOLUTION 4 Sewage effluent with oxygen
	pH	6.00
	pe	7.0
	temp	14.0
	units	umol/L
	Ca	335
	Mg	170
	Na     	2100
	K	240
	Fe	4.5
	Mn	0.4
#	Al	1.9
#	Si	210
	Cl	990    charge
	S(6)	290
	C(4)   	1200
#	Amm	180
	Amm	 1.8000000000e+02
	N(5)	1050     # based on 12 mg/L N (Smith)
#	N(3)	73
#	N(organic)  230
	P	190	# based on average analyses
#	F	33
	O(0)	%        DO     % 
	%           TOC % # 1.6000000000e+03
#	Toc	1667	# based on 20 mg/L average
#	Soc	1000
#	B	56
#	[N2]    100
END
# Equilibrate solution 1
USE solution 1
EQUILIBRIUM_PHASES 1
	Pyrolusite	0.0	0.0007  #based on .18 uM/g Mn
	Fe(OH)3(a)	0.0	0.08    #based on 19 uM/g Fe
#	Siderite	0	0
#	Manganite	0	0
#	Rhodochrosite	0	0
#	Al(OH)3(a)	0.0	0.093   #based on 22 uM/g Al
SAVE solution 1
END
# Use background water in place of rain
USE solution 1
REACTION 1
	NaCl 1
	0 moles
SAVE solution 3
END
EQUILIBRIUM_PHASES 2 Extra manganese
	Pyrolusite	0.0	0.0007   #based on .18 uM/g Mn
	Fe(OH)3(a)	0.0	0.08    #based on 19 uM/g Fe
#	Siderite	0	0
#	Manganite	0	0
#	Rhodochrosite	0	0
END

RATES # REVISED
Remove_N2
      -start
20 k = %   Remove_N2_k %   # 1e-2/(24*3600)
30 rate = k * MOL("N2") * MOL("N2") / (1e-7 + MOL("N2"))
40 moles = rate * TIME * SOLN_VOL
200   save moles
      -end

Strengite
      -start
10    k = %    Remove_P_k %  # 2e-3/(24*3600)
20    rate = -k*TOT("P")*SI("Strengite")/(1 + ABS(SI("Strengite")))
30    moles = rate * TIME * SOLN_VOL
200   save moles
      -end

Vivianite
      -start
10    k = %    Remove_P_k % # 2e-3/(24*3600)
20    rate = -k*TOT("P")*SI("Vivianite")/(1 + ABS(SI("Vivianite")))
30    moles = rate * TIME * SOLN_VOL
200   save moles
      -end

Decay
      -start
10    tToc = tot("Toc")
#12    if (tToc <= 1e-10) then goto 200
40    rate =  %       Decay_k %    # 1.0000000000e-07
60    moles = rate * tToc * time * tToc / (1e-7 + tToc) * SOLN_VOL
#90    if (moles >= tToc) then moles = tToc - 1e-10*tToc
200   save moles
      -end

SOC
      -start
10 REM Before cessation, sorb carbon
20 if (TOTAL_TIME/(365.25*24*3600) >= 1996) THEN GOTO 200
#30 if TOT("Soc") < 1e-9 THEN GOTO 1000
40 k = %    Sorb_SOC_k % # 1e-3/(24*3600)
50 rate = k*TOT("Soc") * TOT("Soc") / (1e-7 + TOT("Soc")) * SOLN_VOL
60 moles = -rate*TIME 		# result is negative, M will increase
70 GOTO 1000
190 REM
200 REM After cessation, react sorbed carbon
#210 if (M <= 0) THEN GOTO 1000
220 ea = TOT("O(0)") + TOT("N(5)")
#230 if ea < 1e-9 THEN GOTO 1000
240 k = 1e-3/(24*3600)
250 rate = k*ea * ea / (1e-7 + ea) * SOLN_VOL
260 moles = rate*TIME   	# result is positive, M will decrease
1000   save moles
      -end
SOC_decomposition
      -start
10 REM After cessation, react sorbed carbon
20 if (TOTAL_TIME/(365.25*24*3600) < 1996) THEN GOTO 1000
210 if (KIN("SOC") <= 0) THEN GOTO 1000
220 ea = TOT("O(0)") + TOT("N(5)")
#230 if ea < 1e-9 THEN GOTO 1000
240 k = %   React_SOC_k % # 1e-3/(24*3600)
250 rate = k*ea * ea / (1e-7 + ea) * SOLN_VOL
260 moles = rate*TIME   	# result is positive, M will decrease
1000   save moles
      -end

END
KINETICS 1
#-cvode
#-step_divide 1.5
Remove_N2
	-M	1
	-formula [N2] 1 N -2

Strengite
	-tol 	1e-8
	-m	0
	-m0	0

Vivianite
	-tol 	1e-8
	-m	0
	-m0	0

Decay
	-formula CH2O(Amm)0.0 1 Toc -1 # now can do it with 1 definition
	-tol 1e-8

SOC
	-M	0
	-formula Soc 1 

SOC_decomposition
	-M	1
	-formula CH2O 1 

KINETICS 2
#-cvode
#-step_divide 1.5
Remove_N2
	-M	1
	-formula [N2] 1 N -2

Strengite
	-m	0
	-m0	0

Vivianite
	-tol 	1e-8
	-m	0
	-m0	0

Decay
	-M	1
	-formula CH2O(Amm)0.0 1 Toc -1 # now can do it with 1 definition

SOC
	-M	0
	-formula Soc 1 

SOC_decomposition
	-M	1
	-formula CH2O 1 

SURFACE 1
	-equil solution 1
#       Fit parameter
  	CationOH       %  Cation_sites %    3.0000000000e-01   4.1500000000e+03
#       Fit parameter
 	AnionOH        %  Cation_sites %    3.0000000000e-01   4.1500000000e+03
END

SELECTED_OUTPUT
	-file 	full3d.sel
	-reset 	false

USER_PUNCH
	-head	pH	PO4	Fe	Mn	NH3	Cl	DOC	NO3	O2	SorbedP	Pyrolusite	Fe(OH)3(a)	Ca	Mg	Na	Alk	S-2	Kin_streng	KIN_viv	SI_sid	SI_stren	SI_viv	EQ_mang	EQ_rhodo	EQ_sid	Soc Kin_Soc	[N2]	N2
	-start
20 PUNCH -la("H+")
30 PUNCH TOT("P")*1e6
40 PUNCH TOT("Fe")*1e6
50 PUNCH TOT("Mn")*1e6
60 PUNCH TOT("Amm")*1e6
100 PUNCH TOT("Cl")*1e6
110 PUNCH TOT("Toc")*1e6
120 PUNCH TOT("N(5)")*1e6
150 PUNCH MOL("O2")*1e6
160 PUNCH (MOL("AnionH2PO4") + MOL("AnionHPO4-") + MOL("AnionPO4-2"))*1e6
190 PUNCH EQUI("Pyrolusite")*1e6
200 PUNCH EQUI("Fe(OH)3(a)")*1e6
210 PUNCH TOT("Ca")*1e6
220 PUNCH TOT("Mg")*1e6
230 PUNCH TOT("Na")*1e6
240 PUNCH ALK*1e6
250 PUNCH TOT("S(-2)")*1e6
300 PUNCH KIN("Strengite")*1e6
305 PUNCH KIN("Vivianite")*1e6
310 PUNCH SI("Siderite")
320 PUNCH SI("Strengite")
330 PUNCH SI("Vivianite")
340 PUNCH EQUI("Manganite")*1e6
350 PUNCH EQUI("Rhodochrosite")*1e6
360 PUNCH EQUI("Siderite")*1e6
370 PUNCH TOT("Soc")*1e6
380 PUNCH KIN("SOC")*1e6
390 PUNCH TOT("[N2]")*1e6
400 PUNCH MOL("N2")*1e6
	-end
END
PRINT
	-status false
	-warning	1
	-reset 	false  # will remove most prints to output file
END


