pcf 
* control data 
restart estimation 
     17   240      1      0     5 
     1     1   single   point   1   0   0 
10.0  -3.0    0.3    0.03     -9  999  LAMFORGIVE DERFORGIVE 
0.2   2.0   1.0e-3 
0.1  noaui 
30   .005  4   4  .005   4 
1    1    1  
* singular value decomposition 
1 
1 5e-7 
1 
* parameter groups 
chem         relative      1.00000E-02  0.0000    switch     2.0000      parabolic
trans        relative      1.00000E-02  0.0000    switch     2.0000      parabolic
* parameter data 
AnionOH2_k	fixed	relative	4.1		1	10	chem	6.667E-01	-1.067E+01	1
AnionHPO4_k	fixed	relative	26.7		1	10	chem	3.333E-01	-7.333E+00	1
Cation_sites	fixed	relative	3450e-6		1	10	chem	6.667E-01	-1.067E+01	1
CationOH2_k	fixed	relative	4.1		1	10	chem	4.000E-01	-7.400E+00	1
Cation_k	fixed	relative	-1.8		1	10	chem	6.667E-01	-1.067E+01	1
Cation_sites	fixed	relative	23000e-6	1	10	chem	6.667E-01	-1.067E+01	1
DO		fixed	relative	250.		1	10	chem	4.444E-01	-8.444E+00	1
Remove_N2_k	fixed	relative	1.16e-7		1	10	chem	4.444E-01	-8.444E+00	1
Effluent_P	fixed	relative	380.		1	10	chem	4.444E-01	-8.444E+00	1
Remove_P_k	fixed	relative	1.16e-8		1	10	chem	4.444E-01	-8.444E+00	1
TOC		fixed	relative	1600.0		1	10	chem	6.667E-01	-1.067E+01	1
Decay_k		fixed	relative	1e-7		1	10	chem	3.333E-01	-7.333E+00	1
SOC		fixed	relative	1000.		1	10	chem	6.667E-01	-1.067E+01	1
Sorb_SOC_k	fixed	relative	1.16e-7		1	10	chem	4.444E-01	-8.444E+00	1
React_SOC_k	fixed	relative	1.16e-7		1	10	chem	4.444E-01	-8.444E+00	1
time_step	fixed	relative	0.25		1	10	trans	0.025		0.25		1
time_change	fixed	relative	1995		1	10	trans	1995		2015		1


* observation groups 
na 
mg
k
ca
si
* observation data 
P_uM_1993_1     0.10003E+03    0.10000E+01             P
P_uM_1993_2     0.10003E+03    0.10000E+01             P
P_uM_1993_3     0.69490E+03    0.10000E+01             P
P_uM_1993_4     0.69637E+03    0.10000E+01             P
P_uM_1993_5     0.10000E-01    0.10000E+01             P
P_uM_1993_6     0.10000E-01    0.10000E+01             P
P_uM_1993_7     0.10000E-01    0.10000E+01             P
P_uM_1993_8     0.10000E-01    0.10000E+01             P
P_uM_1993_9     0.20000E-01    0.10000E+01             P
P_uM_1993_10     0.50000E-01    0.10000E+01             P
P_uM_1993_11     0.40000E-01    0.10000E+01             P
P_uM_1993_12     0.30000E-01    0.10000E+01             P
P_uM_1993_13     0.70000E-01    0.10000E+01             P
P_uM_1993_14     0.50000E-01    0.10000E+01             P
P_uM_1993_15     0.10000E-01    0.10000E+01             P
P_uM_1993_16     0.11000E+01    0.10000E+01             P
P_uM_1993_17     0.77000E+00    0.10000E+01             P
P_uM_1993_18     0.18000E+00    0.10000E+01             P
P_uM_1993_19     0.10000E+00    0.10000E+01             P
P_uM_1993_20     0.40000E-01    0.10000E+01             P
P_uM_1993_21     0.30000E-01    0.10000E+01             P
P_uM_1993_22     0.10000E+01    0.10000E+01             P
P_uM_1993_23     0.89000E+00    0.10000E+01             P
P_uM_1993_24     0.76000E+00    0.10000E+01             P
P_uM_1993_25     0.66000E+00    0.10000E+01             P
P_uM_1993_26     0.64000E+00    0.10000E+01             P
P_uM_1993_27     0.77000E+00    0.10000E+01             P
P_uM_1993_28     0.93000E+00    0.10000E+01             P
P_uM_1993_29     0.77000E+00    0.10000E+01             P
P_uM_1993_30     0.25000E+00    0.10000E+01             P
P_uM_1993_31     0.80000E-01    0.10000E+01             P
P_uM_1993_32     0.10000E+00    0.10000E+01             P
P_uM_1993_33     0.27000E+00    0.10000E+01             P
P_uM_1993_34     0.15000E+00    0.10000E+01             P
P_uM_1993_35     0.80000E-01    0.10000E+01             P
P_uM_1993_36     0.10000E+00    0.10000E+01             P
P_uM_1993_37     0.10000E-01    0.10000E+01             P
P_uM_1993_38     0.10000E-01    0.10000E+01             P
P_uM_1993_39     0.10000E-01    0.10000E+01             P
P_uM_1993_40     0.10000E-01    0.10000E+01             P
P_uM_1993_41     0.20000E-01    0.10000E+01             P
P_uM_1993_42     0.40000E-01    0.10000E+01             P
P_uM_1993_43     0.11000E+01    0.10000E+01             P
P_uM_1993_44     0.13000E+01    0.10000E+01             P
P_uM_1993_45     0.12000E+01    0.10000E+01             P
P_uM_1993_46     0.10000E+01    0.10000E+01             P
P_uM_1993_47     0.12000E+01    0.10000E+01             P
P_uM_1993_48     0.13000E+01    0.10000E+01             P
P_uM_1993_49     0.14000E+01    0.10000E+01             P
P_uM_1993_50     0.13000E+01    0.10000E+01             P
P_uM_1993_51     0.12000E+01    0.10000E+01             P
P_uM_1993_52     0.26244E+03    0.10000E+01             P
P_uM_1993_53     0.26348E+03    0.10000E+01             P
P_uM_1993_54     0.20000E-01    0.10000E+01             P
P_uM_1993_55     0.10000E-01    0.10000E+01             P
P_uM_1993_56     0.10000E-01    0.10000E+01             P
P_uM_1993_57     0.30000E-01    0.10000E+01             P
P_uM_1993_58     0.40000E-01    0.10000E+01             P
P_uM_1993_59     0.30000E-01    0.10000E+01             P
P_uM_1993_60     0.70000E-01    0.10000E+01             P
P_uM_1993_61     0.90000E-01    0.10000E+01             P
P_uM_1993_62     0.10000E+00    0.10000E+01             P
P_uM_1993_63     0.20000E+00    0.10000E+01             P
P_uM_1993_64     0.21000E+00    0.10000E+01             P
P_uM_1993_65     0.21000E+00    0.10000E+01             P
P_uM_1993_66     0.17000E+00    0.10000E+01             P
P_uM_1993_67     0.22000E+00    0.10000E+01             P
P_uM_1993_68     0.21000E+00    0.10000E+01             P
P_uM_1993_69     0.28000E+00    0.10000E+01             P
P_uM_1993_70     0.15000E+00    0.10000E+01             P
P_uM_1993_71     0.80000E-01    0.10000E+01             P
P_uM_1993_72     0.80000E-01    0.10000E+01             P
P_uM_1993_73     0.70000E-01    0.10000E+01             P
P_uM_1993_74     0.70000E-01    0.10000E+01             P
P_uM_1993_75     0.80000E-01    0.10000E+01             P
P_uM_1993_76     0.50000E-01    0.10000E+01             P
P_uM_1993_77     0.60000E-01    0.10000E+01             P
P_uM_1993_78     0.79000E+00    0.10000E+01             P
P_uM_1993_79     0.80000E-01    0.10000E+01             P
P_uM_1993_80     0.10000E-01    0.10000E+01             P
P_uM_1993_81     0.40000E-01    0.10000E+01             P
P_uM_1993_82     0.30000E-01    0.10000E+01             P
P_uM_1993_83     0.10000E-01    0.10000E+01             P
P_uM_1993_84     0.20000E-01    0.10000E+01             P
P_uM_1993_85     0.70000E-01    0.10000E+01             P
P_uM_1993_86     0.50000E+00    0.10000E+01             P
P_uM_1993_87     0.68000E+00    0.10000E+01             P
P_uM_1993_88     0.71000E+00    0.10000E+01             P
P_uM_1993_89     0.43000E+01    0.10000E+01             P
P_uM_1993_90     0.51000E+00    0.10000E+01             P
P_uM_1993_91     0.39000E+00    0.10000E+01             P
P_uM_1993_92     0.35000E+00    0.10000E+01             P
P_uM_1993_93     0.29000E+00    0.10000E+01             P
P_uM_1993_94     0.23000E+00    0.10000E+01             P
P_uM_1993_95     0.16000E+00    0.10000E+01             P
P_uM_1993_96     0.14000E+00    0.10000E+01             P
P_uM_1993_97     0.14000E+00    0.10000E+01             P
P_uM_1993_98     0.38845E+03    0.10000E+01             P
P_uM_1993_99     0.38542E+03    0.10000E+01             P
P_uM_1993_100     0.38510E+03    0.10000E+01             P
P_uM_1993_101     0.90000E-01    0.10000E+01             P
P_uM_1993_102     0.12000E+01    0.10000E+01             P
P_uM_1993_103     0.19000E+01    0.10000E+01             P
P_uM_1993_104     0.15000E+01    0.10000E+01             P
P_uM_1993_105     0.79000E+00    0.10000E+01             P
P_uM_1993_106     0.54000E+00    0.10000E+01             P
P_uM_1993_107     0.41000E+00    0.10000E+01             P
P_uM_1993_108     0.47000E+00    0.10000E+01             P
P_uM_1993_109     0.18000E+01    0.10000E+01             P
P_uM_1993_110     0.13000E+01    0.10000E+01             P
P_uM_1993_111     0.16000E+01    0.10000E+01             P
P_uM_1993_112     0.10000E+01    0.10000E+01             P
P_uM_1993_113     0.37000E+00    0.10000E+01             P
P_uM_1993_114     0.24000E+00    0.10000E+01             P
P_uM_1993_115     0.37233E+03    0.10000E+01             P
P_uM_1993_116     0.37396E+03    0.10000E+01             P
P_uM_1993_117     0.37191E+03    0.10000E+01             P
P_uM_1993_118     0.56010E+03    0.10000E+01             P
P_uM_1993_119     0.30000E-01    0.10000E+01             P
P_uM_1993_120     0.54000E+00    0.10000E+01             P
P_uM_1993_121     0.10000E+00    0.10000E+01             P
P_uM_1993_122     0.11000E+01    0.10000E+01             P
P_uM_1993_123     0.94000E+00    0.10000E+01             P
P_uM_1993_124     0.82000E+00    0.10000E+01             P
P_uM_1993_125     0.79000E+00    0.10000E+01             P
P_uM_1993_126     0.78000E+00    0.10000E+01             P
P_uM_1993_127     0.62000E+00    0.10000E+01             P
P_uM_1993_128     0.55000E+00    0.10000E+01             P
P_uM_1993_129     0.41000E+00    0.10000E+01             P
P_uM_1993_130     0.37000E+00    0.10000E+01             P
P_uM_1993_131     0.34000E+00    0.10000E+01             P
P_uM_1993_132     0.32000E+00    0.10000E+01             P
P_uM_1993_133     0.30000E+00    0.10000E+01             P
P_uM_1993_134     0.27000E+00    0.10000E+01             P
P_uM_1993_135     0.21000E+00    0.10000E+01             P
P_uM_1993_136     0.10000E+00    0.10000E+01             P
P_uM_1993_137     0.10000E-01    0.10000E+01             P
P_uM_1993_138     0.10000E-01    0.10000E+01             P
P_uM_1993_139     0.10000E-01    0.10000E+01             P
P_uM_1993_140     0.10000E-01    0.10000E+01             P
P_uM_1993_141     0.20000E-01    0.10000E+01             P
P_uM_1993_142     0.20000E-01    0.10000E+01             P
P_uM_1993_143     0.30000E-01    0.10000E+01             P
P_uM_1993_144     0.39000E+00    0.10000E+01             P
P_uM_1993_145     0.62000E+00    0.10000E+01             P
P_uM_1993_146     0.93000E+00    0.10000E+01             P
P_uM_1993_147     0.11000E+01    0.10000E+01             P
P_uM_1993_148     0.12000E+01    0.10000E+01             P
P_uM_1993_149     0.13000E+01    0.10000E+01             P
P_uM_1993_150     0.12000E+01    0.10000E+01             P
P_uM_1993_151     0.12000E+01    0.10000E+01             P
P_uM_1993_152     0.20000E-01    0.10000E+01             P
P_uM_1993_153     0.10000E-01    0.10000E+01             P
P_uM_1993_154     0.30000E-01    0.10000E+01             P
P_uM_1993_155     0.10000E-01    0.10000E+01             P
P_uM_1993_156     0.38654E+03    0.10000E+01             P
P_uM_1993_157     0.38654E+03    0.10000E+01             P
P_uM_1993_158     0.10000E-01    0.10000E+01             P
P_uM_1993_159     0.10000E-01    0.10000E+01             P
P_uM_1993_160     0.20000E-01    0.10000E+01             P
P_uM_1993_161     0.10000E-01    0.10000E+01             P
P_uM_1993_162     0.20000E-01    0.10000E+01             P
P_uM_1993_163     0.10000E-01    0.10000E+01             P
P_uM_1993_164     0.10000E-01    0.10000E+01             P
P_uM_1993_165     0.34000E+00    0.10000E+01             P
P_uM_1993_166     0.14000E+01    0.10000E+01             P
P_uM_1993_167     0.17000E+01    0.10000E+01             P
P_uM_1993_168     0.11000E+01    0.10000E+01             P
P_uM_1993_169     0.26000E+00    0.10000E+01             P
P_uM_1993_170     0.15000E+00    0.10000E+01             P
P_uM_1993_171     0.21000E+00    0.10000E+01             P
P_uM_1993_172     0.40000E-01    0.10000E+01             P
P_uM_1993_173     0.60000E-01    0.10000E+01             P
P_uM_1993_174     0.10000E-01    0.10000E+01             P
P_uM_1993_175     0.60000E-01    0.10000E+01             P
P_uM_1993_176     0.30000E-01    0.10000E+01             P
P_uM_1993_177     0.40000E-01    0.10000E+01             P
P_uM_1993_178     0.10000E-01    0.10000E+01             P
P_uM_1993_179     0.20000E-01    0.10000E+01             P
P_uM_1993_180     0.20000E-01    0.10000E+01             P
P_uM_1993_181     0.14000E+00    0.10000E+01             P
P_uM_1993_182     0.43000E+00    0.10000E+01             P
P_uM_1993_183     0.64000E+00    0.10000E+01             P
P_uM_1993_184     0.72000E+00    0.10000E+01             P
P_uM_1993_185     0.79000E+00    0.10000E+01             P
P_uM_1993_186     0.84000E+00    0.10000E+01             P
P_uM_1993_187     0.99000E+00    0.10000E+01             P
P_uM_1993_188     0.91000E+00    0.10000E+01             P
P_uM_1993_189     0.97000E+00    0.10000E+01             P
P_uM_1993_190     0.95000E+00    0.10000E+01             P
P_uM_1993_191     0.15000E+01    0.10000E+01             P
P_uM_1993_192     0.13000E+01    0.10000E+01             P
P_uM_1993_193     0.30000E-01    0.10000E+01             P
P_uM_1993_194     0.97000E+00    0.10000E+01             P
P_uM_1993_195     0.26000E+01    0.10000E+01             P
P_uM_1993_196     0.21000E+01    0.10000E+01             P
P_uM_1993_197     0.10000E+01    0.10000E+01             P
P_uM_1993_198     0.22000E+01    0.10000E+01             P
P_uM_1993_199     0.42000E+01    0.10000E+01             P
P_uM_1993_200     0.56000E+01    0.10000E+01             P
P_uM_1993_201     0.47000E+01    0.10000E+01             P
P_uM_1993_202     0.45000E+01    0.10000E+01             P
P_uM_1993_203     0.45000E+01    0.10000E+01             P
P_uM_1993_204     0.16000E+01    0.10000E+01             P
P_uM_1993_205     0.12000E+01    0.10000E+01             P
P_uM_1993_206     0.16000E+01    0.10000E+01             P
P_uM_1993_207     0.11000E+01    0.10000E+01             P
P_uM_1993_208     0.31206E+03    0.10000E+01             P
P_uM_1993_209     0.25321E+02    0.10000E+01             P
P_uM_1993_210     0.11000E+02    0.10000E+01             P
P_uM_1993_211     0.40000E+01    0.10000E+01             P
P_uM_1993_212     0.47000E+00    0.10000E+01             P
P_uM_1993_213    -0.68878E+03    0.10000E+01             P
P_uM_1993_214     0.15788E+03    0.10000E+01             P
P_uM_1993_215     0.16000E+01    0.10000E+01             P
P_uM_1993_216     0.58000E+00    0.10000E+01             P
P_uM_1993_217     0.14000E+01    0.10000E+01             P
P_uM_1993_218     0.36000E+00    0.10000E+01             P
P_uM_1993_219     0.27000E+01    0.10000E+01             P
P_uM_1993_220     0.21000E+01    0.10000E+01             P
P_uM_1993_221     0.18000E+01    0.10000E+01             P
P_uM_1993_222     0.20000E+01    0.10000E+01             P
P_uM_1993_223     0.21000E+01    0.10000E+01             P
P_uM_1993_224     0.21000E+01    0.10000E+01             P
P_uM_1993_225     0.33000E+01    0.10000E+01             P
P_uM_1993_226     0.41000E+01    0.10000E+01             P
P_uM_1993_227     0.41000E+01    0.10000E+01             P
P_uM_1993_228     0.44000E+01    0.10000E+01             P
P_uM_1993_229     0.32000E+01    0.10000E+01             P
P_uM_1993_230     0.27000E+01    0.10000E+01             P
P_uM_1993_231     0.25000E+01    0.10000E+01             P
P_uM_1993_232     0.24000E+01    0.10000E+01             P
P_uM_1993_233     0.25000E+01    0.10000E+01             P
P_uM_1993_234     0.23000E+01    0.10000E+01             P
P_uM_1993_235     0.21000E+01    0.10000E+01             P
P_uM_1993_236     0.24000E+01    0.10000E+01             P
P_uM_1993_237     0.23000E+01    0.10000E+01             P
P_uM_1993_238     0.17000E+01    0.10000E+01             P
P_uM_1993_239     0.59000E+00    0.10000E+01             P
P_uM_1993_240     0.41000E+00    0.10000E+01             P



* model command line 
@PROJECT_DIR@\01_1D.bat

* model input/output 
@PROJECT_DIR@\@PHAST_ROOT_NAME@.chem.dat.tpl		@PHAST_ROOT_NAME@.chem.dat
@PROJECT_DIR@\@PHAST_ROOT_NAME@.trans.dat.tpl		@PHAST_ROOT_NAME@.trans.dat
@PROJECT_DIR@\01_1D.txt.ins		01_1D.txt 
