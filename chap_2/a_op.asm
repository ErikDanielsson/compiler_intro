
a_op.out:     file format elf64-x86-64


Disassembly of section .init:

0000000000001000 <_init>:
    1000:	f3 0f 1e fa          	endbr64 
    1004:	48 83 ec 08          	sub    rsp,0x8
    1008:	48 8b 05 d9 2f 00 00 	mov    rax,QWORD PTR [rip+0x2fd9]        # 3fe8 <__gmon_start__>
    100f:	48 85 c0             	test   rax,rax
    1012:	74 02                	je     1016 <_init+0x16>
    1014:	ff d0                	call   rax
    1016:	48 83 c4 08          	add    rsp,0x8
    101a:	c3                   	ret    

Disassembly of section .plt:

0000000000001020 <.plt>:
    1020:	ff 35 42 2f 00 00    	push   QWORD PTR [rip+0x2f42]        # 3f68 <_GLOBAL_OFFSET_TABLE_+0x8>
    1026:	f2 ff 25 43 2f 00 00 	bnd jmp QWORD PTR [rip+0x2f43]        # 3f70 <_GLOBAL_OFFSET_TABLE_+0x10>
    102d:	0f 1f 00             	nop    DWORD PTR [rax]
    1030:	f3 0f 1e fa          	endbr64 
    1034:	68 00 00 00 00       	push   0x0
    1039:	f2 e9 e1 ff ff ff    	bnd jmp 1020 <.plt>
    103f:	90                   	nop
    1040:	f3 0f 1e fa          	endbr64 
    1044:	68 01 00 00 00       	push   0x1
    1049:	f2 e9 d1 ff ff ff    	bnd jmp 1020 <.plt>
    104f:	90                   	nop
    1050:	f3 0f 1e fa          	endbr64 
    1054:	68 02 00 00 00       	push   0x2
    1059:	f2 e9 c1 ff ff ff    	bnd jmp 1020 <.plt>
    105f:	90                   	nop
    1060:	f3 0f 1e fa          	endbr64 
    1064:	68 03 00 00 00       	push   0x3
    1069:	f2 e9 b1 ff ff ff    	bnd jmp 1020 <.plt>
    106f:	90                   	nop
    1070:	f3 0f 1e fa          	endbr64 
    1074:	68 04 00 00 00       	push   0x4
    1079:	f2 e9 a1 ff ff ff    	bnd jmp 1020 <.plt>
    107f:	90                   	nop
    1080:	f3 0f 1e fa          	endbr64 
    1084:	68 05 00 00 00       	push   0x5
    1089:	f2 e9 91 ff ff ff    	bnd jmp 1020 <.plt>
    108f:	90                   	nop
    1090:	f3 0f 1e fa          	endbr64 
    1094:	68 06 00 00 00       	push   0x6
    1099:	f2 e9 81 ff ff ff    	bnd jmp 1020 <.plt>
    109f:	90                   	nop
    10a0:	f3 0f 1e fa          	endbr64 
    10a4:	68 07 00 00 00       	push   0x7
    10a9:	f2 e9 71 ff ff ff    	bnd jmp 1020 <.plt>
    10af:	90                   	nop
    10b0:	f3 0f 1e fa          	endbr64 
    10b4:	68 08 00 00 00       	push   0x8
    10b9:	f2 e9 61 ff ff ff    	bnd jmp 1020 <.plt>
    10bf:	90                   	nop
    10c0:	f3 0f 1e fa          	endbr64 
    10c4:	68 09 00 00 00       	push   0x9
    10c9:	f2 e9 51 ff ff ff    	bnd jmp 1020 <.plt>
    10cf:	90                   	nop
    10d0:	f3 0f 1e fa          	endbr64 
    10d4:	68 0a 00 00 00       	push   0xa
    10d9:	f2 e9 41 ff ff ff    	bnd jmp 1020 <.plt>
    10df:	90                   	nop
    10e0:	f3 0f 1e fa          	endbr64 
    10e4:	68 0b 00 00 00       	push   0xb
    10e9:	f2 e9 31 ff ff ff    	bnd jmp 1020 <.plt>
    10ef:	90                   	nop

Disassembly of section .plt.got:

00000000000010f0 <__cxa_finalize@plt>:
    10f0:	f3 0f 1e fa          	endbr64 
    10f4:	f2 ff 25 fd 2e 00 00 	bnd jmp QWORD PTR [rip+0x2efd]        # 3ff8 <__cxa_finalize@GLIBC_2.2.5>
    10fb:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

Disassembly of section .plt.sec:

0000000000001100 <free@plt>:
    1100:	f3 0f 1e fa          	endbr64 
    1104:	f2 ff 25 6d 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e6d]        # 3f78 <free@GLIBC_2.2.5>
    110b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

0000000000001110 <putchar@plt>:
    1110:	f3 0f 1e fa          	endbr64 
    1114:	f2 ff 25 65 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e65]        # 3f80 <putchar@GLIBC_2.2.5>
    111b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

0000000000001120 <strcpy@plt>:
    1120:	f3 0f 1e fa          	endbr64 
    1124:	f2 ff 25 5d 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e5d]        # 3f88 <strcpy@GLIBC_2.2.5>
    112b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

0000000000001130 <puts@plt>:
    1130:	f3 0f 1e fa          	endbr64 
    1134:	f2 ff 25 55 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e55]        # 3f90 <puts@GLIBC_2.2.5>
    113b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

0000000000001140 <strlen@plt>:
    1140:	f3 0f 1e fa          	endbr64 
    1144:	f2 ff 25 4d 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e4d]        # 3f98 <strlen@GLIBC_2.2.5>
    114b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

0000000000001150 <__stack_chk_fail@plt>:
    1150:	f3 0f 1e fa          	endbr64 
    1154:	f2 ff 25 45 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e45]        # 3fa0 <__stack_chk_fail@GLIBC_2.4>
    115b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

0000000000001160 <printf@plt>:
    1160:	f3 0f 1e fa          	endbr64 
    1164:	f2 ff 25 3d 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e3d]        # 3fa8 <printf@GLIBC_2.2.5>
    116b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

0000000000001170 <strcmp@plt>:
    1170:	f3 0f 1e fa          	endbr64 
    1174:	f2 ff 25 35 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e35]        # 3fb0 <strcmp@GLIBC_2.2.5>
    117b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

0000000000001180 <getchar@plt>:
    1180:	f3 0f 1e fa          	endbr64 
    1184:	f2 ff 25 2d 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e2d]        # 3fb8 <getchar@GLIBC_2.2.5>
    118b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

0000000000001190 <malloc@plt>:
    1190:	f3 0f 1e fa          	endbr64 
    1194:	f2 ff 25 25 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e25]        # 3fc0 <malloc@GLIBC_2.2.5>
    119b:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

00000000000011a0 <fwrite@plt>:
    11a0:	f3 0f 1e fa          	endbr64 
    11a4:	f2 ff 25 1d 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e1d]        # 3fc8 <fwrite@GLIBC_2.2.5>
    11ab:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

00000000000011b0 <__ctype_b_loc@plt>:
    11b0:	f3 0f 1e fa          	endbr64 
    11b4:	f2 ff 25 15 2e 00 00 	bnd jmp QWORD PTR [rip+0x2e15]        # 3fd0 <__ctype_b_loc@GLIBC_2.3>
    11bb:	0f 1f 44 00 00       	nop    DWORD PTR [rax+rax*1+0x0]

Disassembly of section .text:

00000000000011c0 <_start>:
    11c0:	f3 0f 1e fa          	endbr64 
    11c4:	31 ed                	xor    ebp,ebp
    11c6:	49 89 d1             	mov    r9,rdx
    11c9:	5e                   	pop    rsi
    11ca:	48 89 e2             	mov    rdx,rsp
    11cd:	48 83 e4 f0          	and    rsp,0xfffffffffffffff0
    11d1:	50                   	push   rax
    11d2:	54                   	push   rsp
    11d3:	4c 8d 05 e6 09 00 00 	lea    r8,[rip+0x9e6]        # 1bc0 <__libc_csu_fini>
    11da:	48 8d 0d 6f 09 00 00 	lea    rcx,[rip+0x96f]        # 1b50 <__libc_csu_init>
    11e1:	48 8d 3d bf 04 00 00 	lea    rdi,[rip+0x4bf]        # 16a7 <main>
    11e8:	ff 15 f2 2d 00 00    	call   QWORD PTR [rip+0x2df2]        # 3fe0 <__libc_start_main@GLIBC_2.2.5>
    11ee:	f4                   	hlt    
    11ef:	90                   	nop

00000000000011f0 <deregister_tm_clones>:
    11f0:	48 8d 3d 21 2e 00 00 	lea    rdi,[rip+0x2e21]        # 4018 <__TMC_END__>
    11f7:	48 8d 05 1a 2e 00 00 	lea    rax,[rip+0x2e1a]        # 4018 <__TMC_END__>
    11fe:	48 39 f8             	cmp    rax,rdi
    1201:	74 15                	je     1218 <deregister_tm_clones+0x28>
    1203:	48 8b 05 ce 2d 00 00 	mov    rax,QWORD PTR [rip+0x2dce]        # 3fd8 <_ITM_deregisterTMCloneTable>
    120a:	48 85 c0             	test   rax,rax
    120d:	74 09                	je     1218 <deregister_tm_clones+0x28>
    120f:	ff e0                	jmp    rax
    1211:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]
    1218:	c3                   	ret    
    1219:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]

0000000000001220 <register_tm_clones>:
    1220:	48 8d 3d f1 2d 00 00 	lea    rdi,[rip+0x2df1]        # 4018 <__TMC_END__>
    1227:	48 8d 35 ea 2d 00 00 	lea    rsi,[rip+0x2dea]        # 4018 <__TMC_END__>
    122e:	48 29 fe             	sub    rsi,rdi
    1231:	48 89 f0             	mov    rax,rsi
    1234:	48 c1 ee 3f          	shr    rsi,0x3f
    1238:	48 c1 f8 03          	sar    rax,0x3
    123c:	48 01 c6             	add    rsi,rax
    123f:	48 d1 fe             	sar    rsi,1
    1242:	74 14                	je     1258 <register_tm_clones+0x38>
    1244:	48 8b 05 a5 2d 00 00 	mov    rax,QWORD PTR [rip+0x2da5]        # 3ff0 <_ITM_registerTMCloneTable>
    124b:	48 85 c0             	test   rax,rax
    124e:	74 08                	je     1258 <register_tm_clones+0x38>
    1250:	ff e0                	jmp    rax
    1252:	66 0f 1f 44 00 00    	nop    WORD PTR [rax+rax*1+0x0]
    1258:	c3                   	ret    
    1259:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]

0000000000001260 <__do_global_dtors_aux>:
    1260:	f3 0f 1e fa          	endbr64 
    1264:	80 3d bd 2d 00 00 00 	cmp    BYTE PTR [rip+0x2dbd],0x0        # 4028 <completed.8059>
    126b:	75 2b                	jne    1298 <__do_global_dtors_aux+0x38>
    126d:	55                   	push   rbp
    126e:	48 83 3d 82 2d 00 00 	cmp    QWORD PTR [rip+0x2d82],0x0        # 3ff8 <__cxa_finalize@GLIBC_2.2.5>
    1275:	00 
    1276:	48 89 e5             	mov    rbp,rsp
    1279:	74 0c                	je     1287 <__do_global_dtors_aux+0x27>
    127b:	48 8b 3d 86 2d 00 00 	mov    rdi,QWORD PTR [rip+0x2d86]        # 4008 <__dso_handle>
    1282:	e8 69 fe ff ff       	call   10f0 <__cxa_finalize@plt>
    1287:	e8 64 ff ff ff       	call   11f0 <deregister_tm_clones>
    128c:	c6 05 95 2d 00 00 01 	mov    BYTE PTR [rip+0x2d95],0x1        # 4028 <completed.8059>
    1293:	5d                   	pop    rbp
    1294:	c3                   	ret    
    1295:	0f 1f 00             	nop    DWORD PTR [rax]
    1298:	c3                   	ret    
    1299:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]

00000000000012a0 <frame_dummy>:
    12a0:	f3 0f 1e fa          	endbr64 
    12a4:	e9 77 ff ff ff       	jmp    1220 <register_tm_clones>

00000000000012a9 <init>:
    12a9:	f3 0f 1e fa          	endbr64 
    12ad:	55                   	push   rbp
    12ae:	48 89 e5             	mov    rbp,rsp
    12b1:	e8 ca fe ff ff       	call   1180 <getchar@plt>
    12b6:	88 05 74 2d 00 00    	mov    BYTE PTR [rip+0x2d74],al        # 4030 <peek>
    12bc:	b8 00 00 00 00       	mov    eax,0x0
    12c1:	e8 4d 05 00 00       	call   1813 <create_SymTab>
    12c6:	48 89 05 6b 2d 00 00 	mov    QWORD PTR [rip+0x2d6b],rax        # 4038 <symboltable>
    12cd:	90                   	nop
    12ce:	5d                   	pop    rbp
    12cf:	c3                   	ret    

00000000000012d0 <Lexer>:
    12d0:	f3 0f 1e fa          	endbr64 
    12d4:	55                   	push   rbp
    12d5:	48 89 e5             	mov    rbp,rsp
    12d8:	48 83 c4 80          	add    rsp,0xffffffffffffff80
    12dc:	64 48 8b 04 25 28 00 	mov    rax,QWORD PTR fs:0x28
    12e3:	00 00 
    12e5:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
    12e9:	31 c0                	xor    eax,eax
    12eb:	0f b6 05 3e 2d 00 00 	movzx  eax,BYTE PTR [rip+0x2d3e]        # 4030 <peek>
    12f2:	3c 20                	cmp    al,0x20
    12f4:	74 27                	je     131d <Lexer+0x4d>
    12f6:	0f b6 05 33 2d 00 00 	movzx  eax,BYTE PTR [rip+0x2d33]        # 4030 <peek>
    12fd:	3c 09                	cmp    al,0x9
    12ff:	74 1c                	je     131d <Lexer+0x4d>
    1301:	0f b6 05 28 2d 00 00 	movzx  eax,BYTE PTR [rip+0x2d28]        # 4030 <peek>
    1308:	3c 0a                	cmp    al,0xa
    130a:	75 1f                	jne    132b <Lexer+0x5b>
    130c:	8b 05 fe 2c 00 00    	mov    eax,DWORD PTR [rip+0x2cfe]        # 4010 <line>
    1312:	83 c0 01             	add    eax,0x1
    1315:	89 05 f5 2c 00 00    	mov    DWORD PTR [rip+0x2cf5],eax        # 4010 <line>
    131b:	eb 01                	jmp    131e <Lexer+0x4e>
    131d:	90                   	nop
    131e:	e8 5d fe ff ff       	call   1180 <getchar@plt>
    1323:	88 05 07 2d 00 00    	mov    BYTE PTR [rip+0x2d07],al        # 4030 <peek>
    1329:	eb c0                	jmp    12eb <Lexer+0x1b>
    132b:	90                   	nop
    132c:	0f b6 05 fd 2c 00 00 	movzx  eax,BYTE PTR [rip+0x2cfd]        # 4030 <peek>
    1333:	3c 2f                	cmp    al,0x2f
    1335:	0f 85 a1 00 00 00    	jne    13dc <Lexer+0x10c>
    133b:	e8 40 fe ff ff       	call   1180 <getchar@plt>
    1340:	88 05 ea 2c 00 00    	mov    BYTE PTR [rip+0x2cea],al        # 4030 <peek>
    1346:	0f b6 05 e3 2c 00 00 	movzx  eax,BYTE PTR [rip+0x2ce3]        # 4030 <peek>
    134d:	3c 2f                	cmp    al,0x2f
    134f:	75 1a                	jne    136b <Lexer+0x9b>
    1351:	eb 0b                	jmp    135e <Lexer+0x8e>
    1353:	e8 28 fe ff ff       	call   1180 <getchar@plt>
    1358:	88 05 d2 2c 00 00    	mov    BYTE PTR [rip+0x2cd2],al        # 4030 <peek>
    135e:	0f b6 05 cb 2c 00 00 	movzx  eax,BYTE PTR [rip+0x2ccb]        # 4030 <peek>
    1365:	3c 0a                	cmp    al,0xa
    1367:	75 ea                	jne    1353 <Lexer+0x83>
    1369:	eb 71                	jmp    13dc <Lexer+0x10c>
    136b:	0f b6 05 be 2c 00 00 	movzx  eax,BYTE PTR [rip+0x2cbe]        # 4030 <peek>
    1372:	3c 2a                	cmp    al,0x2a
    1374:	75 39                	jne    13af <Lexer+0xdf>
    1376:	e8 05 fe ff ff       	call   1180 <getchar@plt>
    137b:	88 05 af 2c 00 00    	mov    BYTE PTR [rip+0x2caf],al        # 4030 <peek>
    1381:	0f b6 05 a8 2c 00 00 	movzx  eax,BYTE PTR [rip+0x2ca8]        # 4030 <peek>
    1388:	3c 2a                	cmp    al,0x2a
    138a:	75 ea                	jne    1376 <Lexer+0xa6>
    138c:	e8 ef fd ff ff       	call   1180 <getchar@plt>
    1391:	88 05 99 2c 00 00    	mov    BYTE PTR [rip+0x2c99],al        # 4030 <peek>
    1397:	0f b6 05 92 2c 00 00 	movzx  eax,BYTE PTR [rip+0x2c92]        # 4030 <peek>
    139e:	3c 2f                	cmp    al,0x2f
    13a0:	75 d4                	jne    1376 <Lexer+0xa6>
    13a2:	e8 d9 fd ff ff       	call   1180 <getchar@plt>
    13a7:	88 05 83 2c 00 00    	mov    BYTE PTR [rip+0x2c83],al        # 4030 <peek>
    13ad:	eb 2d                	jmp    13dc <Lexer+0x10c>
    13af:	bf 04 01 00 00       	mov    edi,0x104
    13b4:	e8 d7 fd ff ff       	call   1190 <malloc@plt>
    13b9:	48 89 45 88          	mov    QWORD PTR [rbp-0x78],rax
    13bd:	48 8b 45 88          	mov    rax,QWORD PTR [rbp-0x78]
    13c1:	48 89 c7             	mov    rdi,rax
    13c4:	e8 8f 03 00 00       	call   1758 <find_ptr_error>
    13c9:	48 8b 45 88          	mov    rax,QWORD PTR [rbp-0x78]
    13cd:	c7 00 2f 00 00 00    	mov    DWORD PTR [rax],0x2f
    13d3:	48 8b 45 88          	mov    rax,QWORD PTR [rbp-0x78]
    13d7:	e9 b5 02 00 00       	jmp    1691 <Lexer+0x3c1>
    13dc:	e8 cf fd ff ff       	call   11b0 <__ctype_b_loc@plt>
    13e1:	48 8b 00             	mov    rax,QWORD PTR [rax]
    13e4:	0f b6 15 45 2c 00 00 	movzx  edx,BYTE PTR [rip+0x2c45]        # 4030 <peek>
    13eb:	48 0f be d2          	movsx  rdx,dl
    13ef:	48 01 d2             	add    rdx,rdx
    13f2:	48 01 d0             	add    rax,rdx
    13f5:	0f b7 00             	movzx  eax,WORD PTR [rax]
    13f8:	0f b7 c0             	movzx  eax,ax
    13fb:	25 00 08 00 00       	and    eax,0x800
    1400:	85 c0                	test   eax,eax
    1402:	0f 84 91 00 00 00    	je     1499 <Lexer+0x1c9>
    1408:	c7 45 80 00 00 00 00 	mov    DWORD PTR [rbp-0x80],0x0
    140f:	8b 55 80             	mov    edx,DWORD PTR [rbp-0x80]
    1412:	89 d0                	mov    eax,edx
    1414:	c1 e0 02             	shl    eax,0x2
    1417:	01 d0                	add    eax,edx
    1419:	01 c0                	add    eax,eax
    141b:	89 c2                	mov    edx,eax
    141d:	0f b6 05 0c 2c 00 00 	movzx  eax,BYTE PTR [rip+0x2c0c]        # 4030 <peek>
    1424:	0f be c0             	movsx  eax,al
    1427:	01 d0                	add    eax,edx
    1429:	83 e8 30             	sub    eax,0x30
    142c:	89 45 80             	mov    DWORD PTR [rbp-0x80],eax
    142f:	e8 4c fd ff ff       	call   1180 <getchar@plt>
    1434:	88 05 f6 2b 00 00    	mov    BYTE PTR [rip+0x2bf6],al        # 4030 <peek>
    143a:	e8 71 fd ff ff       	call   11b0 <__ctype_b_loc@plt>
    143f:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1442:	0f b6 15 e7 2b 00 00 	movzx  edx,BYTE PTR [rip+0x2be7]        # 4030 <peek>
    1449:	48 0f be d2          	movsx  rdx,dl
    144d:	48 01 d2             	add    rdx,rdx
    1450:	48 01 d0             	add    rax,rdx
    1453:	0f b7 00             	movzx  eax,WORD PTR [rax]
    1456:	0f b7 c0             	movzx  eax,ax
    1459:	25 00 08 00 00       	and    eax,0x800
    145e:	85 c0                	test   eax,eax
    1460:	75 ad                	jne    140f <Lexer+0x13f>
    1462:	bf 04 01 00 00       	mov    edi,0x104
    1467:	e8 24 fd ff ff       	call   1190 <malloc@plt>
    146c:	48 89 45 a8          	mov    QWORD PTR [rbp-0x58],rax
    1470:	48 8b 45 a8          	mov    rax,QWORD PTR [rbp-0x58]
    1474:	48 89 c7             	mov    rdi,rax
    1477:	e8 dc 02 00 00       	call   1758 <find_ptr_error>
    147c:	48 8b 45 a8          	mov    rax,QWORD PTR [rbp-0x58]
    1480:	c7 00 00 01 00 00    	mov    DWORD PTR [rax],0x100
    1486:	48 8b 45 a8          	mov    rax,QWORD PTR [rbp-0x58]
    148a:	8b 55 80             	mov    edx,DWORD PTR [rbp-0x80]
    148d:	89 50 04             	mov    DWORD PTR [rax+0x4],edx
    1490:	48 8b 45 a8          	mov    rax,QWORD PTR [rbp-0x58]
    1494:	e9 f8 01 00 00       	jmp    1691 <Lexer+0x3c1>
    1499:	e8 12 fd ff ff       	call   11b0 <__ctype_b_loc@plt>
    149e:	48 8b 00             	mov    rax,QWORD PTR [rax]
    14a1:	0f b6 15 88 2b 00 00 	movzx  edx,BYTE PTR [rip+0x2b88]        # 4030 <peek>
    14a8:	48 0f be d2          	movsx  rdx,dl
    14ac:	48 01 d2             	add    rdx,rdx
    14af:	48 01 d0             	add    rax,rdx
    14b2:	0f b7 00             	movzx  eax,WORD PTR [rax]
    14b5:	0f b7 c0             	movzx  eax,ax
    14b8:	25 00 04 00 00       	and    eax,0x400
    14bd:	85 c0                	test   eax,eax
    14bf:	75 0f                	jne    14d0 <Lexer+0x200>
    14c1:	0f b6 05 68 2b 00 00 	movzx  eax,BYTE PTR [rip+0x2b68]        # 4030 <peek>
    14c8:	3c 5f                	cmp    al,0x5f
    14ca:	0f 85 8c 01 00 00    	jne    165c <Lexer+0x38c>
    14d0:	48 c7 45 b0 00 00 00 	mov    QWORD PTR [rbp-0x50],0x0
    14d7:	00 
    14d8:	48 c7 45 b8 00 00 00 	mov    QWORD PTR [rbp-0x48],0x0
    14df:	00 
    14e0:	48 c7 45 c0 00 00 00 	mov    QWORD PTR [rbp-0x40],0x0
    14e7:	00 
    14e8:	48 c7 45 c8 00 00 00 	mov    QWORD PTR [rbp-0x38],0x0
    14ef:	00 
    14f0:	48 c7 45 d0 00 00 00 	mov    QWORD PTR [rbp-0x30],0x0
    14f7:	00 
    14f8:	48 c7 45 d8 00 00 00 	mov    QWORD PTR [rbp-0x28],0x0
    14ff:	00 
    1500:	48 c7 45 e0 00 00 00 	mov    QWORD PTR [rbp-0x20],0x0
    1507:	00 
    1508:	48 c7 45 e8 00 00 00 	mov    QWORD PTR [rbp-0x18],0x0
    150f:	00 
    1510:	0f b6 05 19 2b 00 00 	movzx  eax,BYTE PTR [rip+0x2b19]        # 4030 <peek>
    1517:	88 45 b0             	mov    BYTE PTR [rbp-0x50],al
    151a:	e8 61 fc ff ff       	call   1180 <getchar@plt>
    151f:	88 05 0b 2b 00 00    	mov    BYTE PTR [rip+0x2b0b],al        # 4030 <peek>
    1525:	c7 45 84 01 00 00 00 	mov    DWORD PTR [rbp-0x7c],0x1
    152c:	eb 1f                	jmp    154d <Lexer+0x27d>
    152e:	0f b6 15 fb 2a 00 00 	movzx  edx,BYTE PTR [rip+0x2afb]        # 4030 <peek>
    1535:	8b 45 84             	mov    eax,DWORD PTR [rbp-0x7c]
    1538:	48 98                	cdqe   
    153a:	88 54 05 b0          	mov    BYTE PTR [rbp+rax*1-0x50],dl
    153e:	e8 3d fc ff ff       	call   1180 <getchar@plt>
    1543:	88 05 e7 2a 00 00    	mov    BYTE PTR [rip+0x2ae7],al        # 4030 <peek>
    1549:	83 45 84 01          	add    DWORD PTR [rbp-0x7c],0x1
    154d:	e8 5e fc ff ff       	call   11b0 <__ctype_b_loc@plt>
    1552:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1555:	0f b6 15 d4 2a 00 00 	movzx  edx,BYTE PTR [rip+0x2ad4]        # 4030 <peek>
    155c:	48 0f be d2          	movsx  rdx,dl
    1560:	48 01 d2             	add    rdx,rdx
    1563:	48 01 d0             	add    rax,rdx
    1566:	0f b7 00             	movzx  eax,WORD PTR [rax]
    1569:	0f b7 c0             	movzx  eax,ax
    156c:	83 e0 08             	and    eax,0x8
    156f:	85 c0                	test   eax,eax
    1571:	74 06                	je     1579 <Lexer+0x2a9>
    1573:	83 7d 84 3e          	cmp    DWORD PTR [rbp-0x7c],0x3e
    1577:	7e b5                	jle    152e <Lexer+0x25e>
    1579:	e8 32 fc ff ff       	call   11b0 <__ctype_b_loc@plt>
    157e:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1581:	0f b6 15 a8 2a 00 00 	movzx  edx,BYTE PTR [rip+0x2aa8]        # 4030 <peek>
    1588:	48 0f be d2          	movsx  rdx,dl
    158c:	48 01 d2             	add    rdx,rdx
    158f:	48 01 d0             	add    rax,rdx
    1592:	0f b7 00             	movzx  eax,WORD PTR [rax]
    1595:	0f b7 c0             	movzx  eax,ax
    1598:	83 e0 08             	and    eax,0x8
    159b:	85 c0                	test   eax,eax
    159d:	74 2a                	je     15c9 <Lexer+0x2f9>
    159f:	48 8b 05 7a 2a 00 00 	mov    rax,QWORD PTR [rip+0x2a7a]        # 4020 <stderr@@GLIBC_2.2.5>
    15a6:	48 89 c1             	mov    rcx,rax
    15a9:	ba 1a 00 00 00       	mov    edx,0x1a
    15ae:	be 01 00 00 00       	mov    esi,0x1
    15b3:	48 8d 3d 4a 0a 00 00 	lea    rdi,[rip+0xa4a]        # 2004 <_IO_stdin_used+0x4>
    15ba:	e8 e1 fb ff ff       	call   11a0 <fwrite@plt>
    15bf:	b8 00 00 00 00       	mov    eax,0x0
    15c4:	e9 c8 00 00 00       	jmp    1691 <Lexer+0x3c1>
    15c9:	83 45 84 01          	add    DWORD PTR [rbp-0x7c],0x1
    15cd:	8b 45 84             	mov    eax,DWORD PTR [rbp-0x7c]
    15d0:	48 98                	cdqe   
    15d2:	c6 44 05 b0 00       	mov    BYTE PTR [rbp+rax*1-0x50],0x0
    15d7:	48 8b 05 5a 2a 00 00 	mov    rax,QWORD PTR [rip+0x2a5a]        # 4038 <symboltable>
    15de:	48 8d 55 b0          	lea    rdx,[rbp-0x50]
    15e2:	48 89 d6             	mov    rsi,rdx
    15e5:	48 89 c7             	mov    rdi,rax
    15e8:	e8 0c 04 00 00       	call   19f9 <SymTab_get>
    15ed:	48 89 45 98          	mov    QWORD PTR [rbp-0x68],rax
    15f1:	48 83 7d 98 00       	cmp    QWORD PTR [rbp-0x68],0x0
    15f6:	74 09                	je     1601 <Lexer+0x331>
    15f8:	48 8b 45 98          	mov    rax,QWORD PTR [rbp-0x68]
    15fc:	e9 90 00 00 00       	jmp    1691 <Lexer+0x3c1>
    1601:	bf 04 01 00 00       	mov    edi,0x104
    1606:	e8 85 fb ff ff       	call   1190 <malloc@plt>
    160b:	48 89 45 a0          	mov    QWORD PTR [rbp-0x60],rax
    160f:	48 8b 45 a0          	mov    rax,QWORD PTR [rbp-0x60]
    1613:	48 89 c7             	mov    rdi,rax
    1616:	e8 3d 01 00 00       	call   1758 <find_ptr_error>
    161b:	48 8b 45 a0          	mov    rax,QWORD PTR [rbp-0x60]
    161f:	c7 00 01 01 00 00    	mov    DWORD PTR [rax],0x101
    1625:	48 8b 45 a0          	mov    rax,QWORD PTR [rbp-0x60]
    1629:	48 8d 50 04          	lea    rdx,[rax+0x4]
    162d:	48 8d 45 b0          	lea    rax,[rbp-0x50]
    1631:	48 89 c6             	mov    rsi,rax
    1634:	48 89 d7             	mov    rdi,rdx
    1637:	e8 e4 fa ff ff       	call   1120 <strcpy@plt>
    163c:	48 8b 05 f5 29 00 00 	mov    rax,QWORD PTR [rip+0x29f5]        # 4038 <symboltable>
    1643:	48 8b 55 a0          	mov    rdx,QWORD PTR [rbp-0x60]
    1647:	48 8d 4d b0          	lea    rcx,[rbp-0x50]
    164b:	48 89 ce             	mov    rsi,rcx
    164e:	48 89 c7             	mov    rdi,rax
    1651:	e8 a1 02 00 00       	call   18f7 <SymTab_set>
    1656:	48 8b 45 a0          	mov    rax,QWORD PTR [rbp-0x60]
    165a:	eb 35                	jmp    1691 <Lexer+0x3c1>
    165c:	bf 04 01 00 00       	mov    edi,0x104
    1661:	e8 2a fb ff ff       	call   1190 <malloc@plt>
    1666:	48 89 45 90          	mov    QWORD PTR [rbp-0x70],rax
    166a:	48 8b 45 90          	mov    rax,QWORD PTR [rbp-0x70]
    166e:	48 89 c7             	mov    rdi,rax
    1671:	e8 e2 00 00 00       	call   1758 <find_ptr_error>
    1676:	0f b6 05 b3 29 00 00 	movzx  eax,BYTE PTR [rip+0x29b3]        # 4030 <peek>
    167d:	0f be d0             	movsx  edx,al
    1680:	48 8b 45 90          	mov    rax,QWORD PTR [rbp-0x70]
    1684:	89 10                	mov    DWORD PTR [rax],edx
    1686:	c6 05 a3 29 00 00 20 	mov    BYTE PTR [rip+0x29a3],0x20        # 4030 <peek>
    168d:	48 8b 45 90          	mov    rax,QWORD PTR [rbp-0x70]
    1691:	48 8b 4d f8          	mov    rcx,QWORD PTR [rbp-0x8]
    1695:	64 48 33 0c 25 28 00 	xor    rcx,QWORD PTR fs:0x28
    169c:	00 00 
    169e:	74 05                	je     16a5 <Lexer+0x3d5>
    16a0:	e8 ab fa ff ff       	call   1150 <__stack_chk_fail@plt>
    16a5:	c9                   	leave  
    16a6:	c3                   	ret    

00000000000016a7 <main>:
    16a7:	f3 0f 1e fa          	endbr64 
    16ab:	55                   	push   rbp
    16ac:	48 89 e5             	mov    rbp,rsp
    16af:	48 83 ec 10          	sub    rsp,0x10
    16b3:	b8 00 00 00 00       	mov    eax,0x0
    16b8:	e8 ec fb ff ff       	call   12a9 <init>
    16bd:	b8 00 00 00 00       	mov    eax,0x0
    16c2:	e8 09 fc ff ff       	call   12d0 <Lexer>
    16c7:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
    16cb:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    16cf:	48 89 c7             	mov    rdi,rax
    16d2:	e8 81 00 00 00       	call   1758 <find_ptr_error>
    16d7:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    16db:	8b 00                	mov    eax,DWORD PTR [rax]
    16dd:	3d 01 01 00 00       	cmp    eax,0x101
    16e2:	75 1e                	jne    1702 <main+0x5b>
    16e4:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    16e8:	48 83 c0 04          	add    rax,0x4
    16ec:	48 89 c6             	mov    rsi,rax
    16ef:	48 8d 3d 29 09 00 00 	lea    rdi,[rip+0x929]        # 201f <_IO_stdin_used+0x1f>
    16f6:	b8 00 00 00 00       	mov    eax,0x0
    16fb:	e8 60 fa ff ff       	call   1160 <printf@plt>
    1700:	eb 42                	jmp    1744 <main+0x9d>
    1702:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1706:	8b 00                	mov    eax,DWORD PTR [rax]
    1708:	3d 00 01 00 00       	cmp    eax,0x100
    170d:	75 1c                	jne    172b <main+0x84>
    170f:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1713:	8b 40 04             	mov    eax,DWORD PTR [rax+0x4]
    1716:	89 c6                	mov    esi,eax
    1718:	48 8d 3d 0c 09 00 00 	lea    rdi,[rip+0x90c]        # 202b <_IO_stdin_used+0x2b>
    171f:	b8 00 00 00 00       	mov    eax,0x0
    1724:	e8 37 fa ff ff       	call   1160 <printf@plt>
    1729:	eb 19                	jmp    1744 <main+0x9d>
    172b:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    172f:	8b 00                	mov    eax,DWORD PTR [rax]
    1731:	89 c6                	mov    esi,eax
    1733:	48 8d 3d fa 08 00 00 	lea    rdi,[rip+0x8fa]        # 2034 <_IO_stdin_used+0x34>
    173a:	b8 00 00 00 00       	mov    eax,0x0
    173f:	e8 1c fa ff ff       	call   1160 <printf@plt>
    1744:	48 8b 05 ed 28 00 00 	mov    rax,QWORD PTR [rip+0x28ed]        # 4038 <symboltable>
    174b:	48 89 c7             	mov    rdi,rax
    174e:	e8 21 03 00 00       	call   1a74 <SymTab_dump>
    1753:	e9 65 ff ff ff       	jmp    16bd <main+0x16>

0000000000001758 <find_ptr_error>:
    1758:	f3 0f 1e fa          	endbr64 
    175c:	55                   	push   rbp
    175d:	48 89 e5             	mov    rbp,rsp
    1760:	48 83 ec 10          	sub    rsp,0x10
    1764:	48 89 7d f8          	mov    QWORD PTR [rbp-0x8],rdi
    1768:	48 83 7d f8 00       	cmp    QWORD PTR [rbp-0x8],0x0
    176d:	75 20                	jne    178f <find_ptr_error+0x37>
    176f:	48 8b 05 aa 28 00 00 	mov    rax,QWORD PTR [rip+0x28aa]        # 4020 <stderr@@GLIBC_2.2.5>
    1776:	48 89 c1             	mov    rcx,rax
    1779:	ba 26 00 00 00       	mov    edx,0x26
    177e:	be 01 00 00 00       	mov    esi,0x1
    1783:	48 8d 3d be 08 00 00 	lea    rdi,[rip+0x8be]        # 2048 <_IO_stdin_used+0x48>
    178a:	e8 11 fa ff ff       	call   11a0 <fwrite@plt>
    178f:	90                   	nop
    1790:	c9                   	leave  
    1791:	c3                   	ret    

0000000000001792 <hash>:
    1792:	f3 0f 1e fa          	endbr64 
    1796:	55                   	push   rbp
    1797:	48 89 e5             	mov    rbp,rsp
    179a:	48 83 ec 30          	sub    rsp,0x30
    179e:	48 89 7d d8          	mov    QWORD PTR [rbp-0x28],rdi
    17a2:	48 8b 45 d8          	mov    rax,QWORD PTR [rbp-0x28]
    17a6:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
    17aa:	48 8b 45 d8          	mov    rax,QWORD PTR [rbp-0x28]
    17ae:	48 89 c7             	mov    rdi,rax
    17b1:	e8 8a f9 ff ff       	call   1140 <strlen@plt>
    17b6:	89 45 f4             	mov    DWORD PTR [rbp-0xc],eax
    17b9:	c7 45 ec c5 9d 1c 81 	mov    DWORD PTR [rbp-0x14],0x811c9dc5
    17c0:	c7 45 f0 00 00 00 00 	mov    DWORD PTR [rbp-0x10],0x0
    17c7:	eb 23                	jmp    17ec <hash+0x5a>
    17c9:	8b 45 f0             	mov    eax,DWORD PTR [rbp-0x10]
    17cc:	48 63 d0             	movsxd rdx,eax
    17cf:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    17d3:	48 01 d0             	add    rax,rdx
    17d6:	0f b6 00             	movzx  eax,BYTE PTR [rax]
    17d9:	0f be c0             	movsx  eax,al
    17dc:	33 45 ec             	xor    eax,DWORD PTR [rbp-0x14]
    17df:	69 c0 93 01 00 01    	imul   eax,eax,0x1000193
    17e5:	89 45 ec             	mov    DWORD PTR [rbp-0x14],eax
    17e8:	83 45 f0 01          	add    DWORD PTR [rbp-0x10],0x1
    17ec:	8b 45 f0             	mov    eax,DWORD PTR [rbp-0x10]
    17ef:	39 45 f4             	cmp    DWORD PTR [rbp-0xc],eax
    17f2:	77 d5                	ja     17c9 <hash+0x37>
    17f4:	8b 55 ec             	mov    edx,DWORD PTR [rbp-0x14]
    17f7:	89 d0                	mov    eax,edx
    17f9:	48 69 c0 d3 4d 62 10 	imul   rax,rax,0x10624dd3
    1800:	48 c1 e8 20          	shr    rax,0x20
    1804:	c1 e8 06             	shr    eax,0x6
    1807:	69 c0 e8 03 00 00    	imul   eax,eax,0x3e8
    180d:	29 c2                	sub    edx,eax
    180f:	89 d0                	mov    eax,edx
    1811:	c9                   	leave  
    1812:	c3                   	ret    

0000000000001813 <create_SymTab>:
    1813:	f3 0f 1e fa          	endbr64 
    1817:	55                   	push   rbp
    1818:	48 89 e5             	mov    rbp,rsp
    181b:	48 83 ec 10          	sub    rsp,0x10
    181f:	bf 08 00 00 00       	mov    edi,0x8
    1824:	e8 67 f9 ff ff       	call   1190 <malloc@plt>
    1829:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
    182d:	bf 40 1f 00 00       	mov    edi,0x1f40
    1832:	e8 59 f9 ff ff       	call   1190 <malloc@plt>
    1837:	48 89 c2             	mov    rdx,rax
    183a:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    183e:	48 89 10             	mov    QWORD PTR [rax],rdx
    1841:	c7 45 f4 00 00 00 00 	mov    DWORD PTR [rbp-0xc],0x0
    1848:	eb 1f                	jmp    1869 <create_SymTab+0x56>
    184a:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    184e:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1851:	8b 55 f4             	mov    edx,DWORD PTR [rbp-0xc]
    1854:	48 63 d2             	movsxd rdx,edx
    1857:	48 c1 e2 03          	shl    rdx,0x3
    185b:	48 01 d0             	add    rax,rdx
    185e:	48 c7 00 00 00 00 00 	mov    QWORD PTR [rax],0x0
    1865:	83 45 f4 01          	add    DWORD PTR [rbp-0xc],0x1
    1869:	81 7d f4 e7 03 00 00 	cmp    DWORD PTR [rbp-0xc],0x3e7
    1870:	7e d8                	jle    184a <create_SymTab+0x37>
    1872:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1876:	c9                   	leave  
    1877:	c3                   	ret    

0000000000001878 <SymTab_pair>:
    1878:	f3 0f 1e fa          	endbr64 
    187c:	55                   	push   rbp
    187d:	48 89 e5             	mov    rbp,rsp
    1880:	48 83 ec 20          	sub    rsp,0x20
    1884:	48 89 7d e8          	mov    QWORD PTR [rbp-0x18],rdi
    1888:	48 89 75 e0          	mov    QWORD PTR [rbp-0x20],rsi
    188c:	bf 08 00 00 00       	mov    edi,0x8
    1891:	e8 fa f8 ff ff       	call   1190 <malloc@plt>
    1896:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
    189a:	bf 09 00 00 00       	mov    edi,0x9
    189f:	e8 ec f8 ff ff       	call   1190 <malloc@plt>
    18a4:	48 89 c2             	mov    rdx,rax
    18a7:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    18ab:	48 89 10             	mov    QWORD PTR [rax],rdx
    18ae:	bf 08 00 00 00       	mov    edi,0x8
    18b3:	e8 d8 f8 ff ff       	call   1190 <malloc@plt>
    18b8:	48 89 c2             	mov    rdx,rax
    18bb:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    18bf:	48 89 50 08          	mov    QWORD PTR [rax+0x8],rdx
    18c3:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    18c7:	48 8b 00             	mov    rax,QWORD PTR [rax]
    18ca:	48 8b 55 e8          	mov    rdx,QWORD PTR [rbp-0x18]
    18ce:	48 89 d6             	mov    rsi,rdx
    18d1:	48 89 c7             	mov    rdi,rax
    18d4:	e8 47 f8 ff ff       	call   1120 <strcpy@plt>
    18d9:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    18dd:	48 8b 55 e0          	mov    rdx,QWORD PTR [rbp-0x20]
    18e1:	48 89 50 08          	mov    QWORD PTR [rax+0x8],rdx
    18e5:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    18e9:	48 c7 40 10 00 00 00 	mov    QWORD PTR [rax+0x10],0x0
    18f0:	00 
    18f1:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    18f5:	c9                   	leave  
    18f6:	c3                   	ret    

00000000000018f7 <SymTab_set>:
    18f7:	f3 0f 1e fa          	endbr64 
    18fb:	55                   	push   rbp
    18fc:	48 89 e5             	mov    rbp,rsp
    18ff:	53                   	push   rbx
    1900:	48 83 ec 48          	sub    rsp,0x48
    1904:	48 89 7d c8          	mov    QWORD PTR [rbp-0x38],rdi
    1908:	48 89 75 c0          	mov    QWORD PTR [rbp-0x40],rsi
    190c:	48 89 55 b8          	mov    QWORD PTR [rbp-0x48],rdx
    1910:	48 8b 45 c0          	mov    rax,QWORD PTR [rbp-0x40]
    1914:	48 89 c7             	mov    rdi,rax
    1917:	e8 76 fe ff ff       	call   1792 <hash>
    191c:	89 45 dc             	mov    DWORD PTR [rbp-0x24],eax
    191f:	48 8b 45 c8          	mov    rax,QWORD PTR [rbp-0x38]
    1923:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1926:	8b 55 dc             	mov    edx,DWORD PTR [rbp-0x24]
    1929:	48 c1 e2 03          	shl    rdx,0x3
    192d:	48 01 d0             	add    rax,rdx
    1930:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1933:	48 89 45 e0          	mov    QWORD PTR [rbp-0x20],rax
    1937:	48 83 7d e0 00       	cmp    QWORD PTR [rbp-0x20],0x0
    193c:	0f 85 8e 00 00 00    	jne    19d0 <SymTab_set+0xd9>
    1942:	48 8b 45 c8          	mov    rax,QWORD PTR [rbp-0x38]
    1946:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1949:	8b 55 dc             	mov    edx,DWORD PTR [rbp-0x24]
    194c:	48 c1 e2 03          	shl    rdx,0x3
    1950:	48 8d 1c 10          	lea    rbx,[rax+rdx*1]
    1954:	48 8b 55 b8          	mov    rdx,QWORD PTR [rbp-0x48]
    1958:	48 8b 45 c0          	mov    rax,QWORD PTR [rbp-0x40]
    195c:	48 89 d6             	mov    rsi,rdx
    195f:	48 89 c7             	mov    rdi,rax
    1962:	e8 11 ff ff ff       	call   1878 <SymTab_pair>
    1967:	48 89 03             	mov    QWORD PTR [rbx],rax
    196a:	e9 83 00 00 00       	jmp    19f2 <SymTab_set+0xfb>
    196f:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
    1973:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1976:	48 8b 55 c0          	mov    rdx,QWORD PTR [rbp-0x40]
    197a:	48 89 d6             	mov    rsi,rdx
    197d:	48 89 c7             	mov    rdi,rax
    1980:	e8 eb f7 ff ff       	call   1170 <strcmp@plt>
    1985:	85 c0                	test   eax,eax
    1987:	75 33                	jne    19bc <SymTab_set+0xc5>
    1989:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
    198d:	48 8b 40 08          	mov    rax,QWORD PTR [rax+0x8]
    1991:	48 89 c7             	mov    rdi,rax
    1994:	e8 67 f7 ff ff       	call   1100 <free@plt>
    1999:	bf 08 00 00 00       	mov    edi,0x8
    199e:	e8 ed f7 ff ff       	call   1190 <malloc@plt>
    19a3:	48 89 c2             	mov    rdx,rax
    19a6:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
    19aa:	48 89 50 08          	mov    QWORD PTR [rax+0x8],rdx
    19ae:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
    19b2:	48 8b 55 b8          	mov    rdx,QWORD PTR [rbp-0x48]
    19b6:	48 89 50 08          	mov    QWORD PTR [rax+0x8],rdx
    19ba:	eb 36                	jmp    19f2 <SymTab_set+0xfb>
    19bc:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
    19c0:	48 89 45 e8          	mov    QWORD PTR [rbp-0x18],rax
    19c4:	48 8b 45 e8          	mov    rax,QWORD PTR [rbp-0x18]
    19c8:	48 8b 40 10          	mov    rax,QWORD PTR [rax+0x10]
    19cc:	48 89 45 e0          	mov    QWORD PTR [rbp-0x20],rax
    19d0:	48 83 7d e0 00       	cmp    QWORD PTR [rbp-0x20],0x0
    19d5:	75 98                	jne    196f <SymTab_set+0x78>
    19d7:	48 8b 55 b8          	mov    rdx,QWORD PTR [rbp-0x48]
    19db:	48 8b 45 c0          	mov    rax,QWORD PTR [rbp-0x40]
    19df:	48 89 d6             	mov    rsi,rdx
    19e2:	48 89 c7             	mov    rdi,rax
    19e5:	e8 8e fe ff ff       	call   1878 <SymTab_pair>
    19ea:	48 8b 55 e8          	mov    rdx,QWORD PTR [rbp-0x18]
    19ee:	48 89 42 10          	mov    QWORD PTR [rdx+0x10],rax
    19f2:	48 83 c4 48          	add    rsp,0x48
    19f6:	5b                   	pop    rbx
    19f7:	5d                   	pop    rbp
    19f8:	c3                   	ret    

00000000000019f9 <SymTab_get>:
    19f9:	f3 0f 1e fa          	endbr64 
    19fd:	55                   	push   rbp
    19fe:	48 89 e5             	mov    rbp,rsp
    1a01:	48 83 ec 20          	sub    rsp,0x20
    1a05:	48 89 7d e8          	mov    QWORD PTR [rbp-0x18],rdi
    1a09:	48 89 75 e0          	mov    QWORD PTR [rbp-0x20],rsi
    1a0d:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
    1a11:	48 89 c7             	mov    rdi,rax
    1a14:	e8 79 fd ff ff       	call   1792 <hash>
    1a19:	89 45 f4             	mov    DWORD PTR [rbp-0xc],eax
    1a1c:	48 8b 45 e8          	mov    rax,QWORD PTR [rbp-0x18]
    1a20:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1a23:	8b 55 f4             	mov    edx,DWORD PTR [rbp-0xc]
    1a26:	48 c1 e2 03          	shl    rdx,0x3
    1a2a:	48 01 d0             	add    rax,rdx
    1a2d:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1a30:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
    1a34:	eb 30                	jmp    1a66 <SymTab_get+0x6d>
    1a36:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1a3a:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1a3d:	48 8b 55 e0          	mov    rdx,QWORD PTR [rbp-0x20]
    1a41:	48 89 d6             	mov    rsi,rdx
    1a44:	48 89 c7             	mov    rdi,rax
    1a47:	e8 24 f7 ff ff       	call   1170 <strcmp@plt>
    1a4c:	85 c0                	test   eax,eax
    1a4e:	75 0a                	jne    1a5a <SymTab_get+0x61>
    1a50:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1a54:	48 8b 40 08          	mov    rax,QWORD PTR [rax+0x8]
    1a58:	eb 18                	jmp    1a72 <SymTab_get+0x79>
    1a5a:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1a5e:	48 8b 40 10          	mov    rax,QWORD PTR [rax+0x10]
    1a62:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
    1a66:	48 83 7d f8 00       	cmp    QWORD PTR [rbp-0x8],0x0
    1a6b:	75 c9                	jne    1a36 <SymTab_get+0x3d>
    1a6d:	b8 00 00 00 00       	mov    eax,0x0
    1a72:	c9                   	leave  
    1a73:	c3                   	ret    

0000000000001a74 <SymTab_dump>:
    1a74:	f3 0f 1e fa          	endbr64 
    1a78:	55                   	push   rbp
    1a79:	48 89 e5             	mov    rbp,rsp
    1a7c:	48 83 ec 20          	sub    rsp,0x20
    1a80:	48 89 7d e8          	mov    QWORD PTR [rbp-0x18],rdi
    1a84:	bf 0a 00 00 00       	mov    edi,0xa
    1a89:	e8 82 f6 ff ff       	call   1110 <putchar@plt>
    1a8e:	48 8d 3d da 05 00 00 	lea    rdi,[rip+0x5da]        # 206f <_IO_stdin_used+0x6f>
    1a95:	e8 96 f6 ff ff       	call   1130 <puts@plt>
    1a9a:	c7 45 f4 00 00 00 00 	mov    DWORD PTR [rbp-0xc],0x0
    1aa1:	e9 8c 00 00 00       	jmp    1b32 <SymTab_dump+0xbe>
    1aa6:	48 8b 45 e8          	mov    rax,QWORD PTR [rbp-0x18]
    1aaa:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1aad:	8b 55 f4             	mov    edx,DWORD PTR [rbp-0xc]
    1ab0:	48 63 d2             	movsxd rdx,edx
    1ab3:	48 c1 e2 03          	shl    rdx,0x3
    1ab7:	48 01 d0             	add    rax,rdx
    1aba:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1abd:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
    1ac1:	48 83 7d f8 00       	cmp    QWORD PTR [rbp-0x8],0x0
    1ac6:	74 65                	je     1b2d <SymTab_dump+0xb9>
    1ac8:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
    1acb:	89 c6                	mov    esi,eax
    1acd:	48 8d 3d a9 05 00 00 	lea    rdi,[rip+0x5a9]        # 207d <_IO_stdin_used+0x7d>
    1ad4:	b8 00 00 00 00       	mov    eax,0x0
    1ad9:	e8 82 f6 ff ff       	call   1160 <printf@plt>
    1ade:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1ae2:	48 8b 40 08          	mov    rax,QWORD PTR [rax+0x8]
    1ae6:	48 8d 50 04          	lea    rdx,[rax+0x4]
    1aea:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1aee:	48 8b 00             	mov    rax,QWORD PTR [rax]
    1af1:	48 89 c6             	mov    rsi,rax
    1af4:	48 8d 3d 88 05 00 00 	lea    rdi,[rip+0x588]        # 2083 <_IO_stdin_used+0x83>
    1afb:	b8 00 00 00 00       	mov    eax,0x0
    1b00:	e8 5b f6 ff ff       	call   1160 <printf@plt>
    1b05:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1b09:	48 8b 40 10          	mov    rax,QWORD PTR [rax+0x10]
    1b0d:	48 85 c0             	test   rax,rax
    1b10:	74 0e                	je     1b20 <SymTab_dump+0xac>
    1b12:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
    1b16:	48 8b 40 10          	mov    rax,QWORD PTR [rax+0x10]
    1b1a:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
    1b1e:	eb be                	jmp    1ade <SymTab_dump+0x6a>
    1b20:	90                   	nop
    1b21:	bf 0a 00 00 00       	mov    edi,0xa
    1b26:	e8 e5 f5 ff ff       	call   1110 <putchar@plt>
    1b2b:	eb 01                	jmp    1b2e <SymTab_dump+0xba>
    1b2d:	90                   	nop
    1b2e:	83 45 f4 01          	add    DWORD PTR [rbp-0xc],0x1
    1b32:	81 7d f4 e7 03 00 00 	cmp    DWORD PTR [rbp-0xc],0x3e7
    1b39:	0f 8e 67 ff ff ff    	jle    1aa6 <SymTab_dump+0x32>
    1b3f:	bf 0a 00 00 00       	mov    edi,0xa
    1b44:	e8 c7 f5 ff ff       	call   1110 <putchar@plt>
    1b49:	90                   	nop
    1b4a:	c9                   	leave  
    1b4b:	c3                   	ret    
    1b4c:	0f 1f 40 00          	nop    DWORD PTR [rax+0x0]

0000000000001b50 <__libc_csu_init>:
    1b50:	f3 0f 1e fa          	endbr64 
    1b54:	41 57                	push   r15
    1b56:	4c 8d 3d 03 22 00 00 	lea    r15,[rip+0x2203]        # 3d60 <__frame_dummy_init_array_entry>
    1b5d:	41 56                	push   r14
    1b5f:	49 89 d6             	mov    r14,rdx
    1b62:	41 55                	push   r13
    1b64:	49 89 f5             	mov    r13,rsi
    1b67:	41 54                	push   r12
    1b69:	41 89 fc             	mov    r12d,edi
    1b6c:	55                   	push   rbp
    1b6d:	48 8d 2d f4 21 00 00 	lea    rbp,[rip+0x21f4]        # 3d68 <__do_global_dtors_aux_fini_array_entry>
    1b74:	53                   	push   rbx
    1b75:	4c 29 fd             	sub    rbp,r15
    1b78:	48 83 ec 08          	sub    rsp,0x8
    1b7c:	e8 7f f4 ff ff       	call   1000 <_init>
    1b81:	48 c1 fd 03          	sar    rbp,0x3
    1b85:	74 1f                	je     1ba6 <__libc_csu_init+0x56>
    1b87:	31 db                	xor    ebx,ebx
    1b89:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]
    1b90:	4c 89 f2             	mov    rdx,r14
    1b93:	4c 89 ee             	mov    rsi,r13
    1b96:	44 89 e7             	mov    edi,r12d
    1b99:	41 ff 14 df          	call   QWORD PTR [r15+rbx*8]
    1b9d:	48 83 c3 01          	add    rbx,0x1
    1ba1:	48 39 dd             	cmp    rbp,rbx
    1ba4:	75 ea                	jne    1b90 <__libc_csu_init+0x40>
    1ba6:	48 83 c4 08          	add    rsp,0x8
    1baa:	5b                   	pop    rbx
    1bab:	5d                   	pop    rbp
    1bac:	41 5c                	pop    r12
    1bae:	41 5d                	pop    r13
    1bb0:	41 5e                	pop    r14
    1bb2:	41 5f                	pop    r15
    1bb4:	c3                   	ret    
    1bb5:	66 66 2e 0f 1f 84 00 	data16 nop WORD PTR cs:[rax+rax*1+0x0]
    1bbc:	00 00 00 00 

0000000000001bc0 <__libc_csu_fini>:
    1bc0:	f3 0f 1e fa          	endbr64 
    1bc4:	c3                   	ret    

Disassembly of section .fini:

0000000000001bc8 <_fini>:
    1bc8:	f3 0f 1e fa          	endbr64 
    1bcc:	48 83 ec 08          	sub    rsp,0x8
    1bd0:	48 83 c4 08          	add    rsp,0x8
    1bd4:	c3                   	ret    
