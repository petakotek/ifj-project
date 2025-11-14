Executing instruction: JUMP at line: 2 with the following arguments:
Executing instruction: LABEL at line: 3 with the following arguments:
Executing instruction: CREATEFRAME at line: 4 with the following arguments:
Executing instruction: PUSHFRAME at line: 5 with the following arguments:
Executing instruction: DEFVAR at line: 6 with the following arguments:
Dest: LF@retval
Executing instruction: DEFVAR at line: 7 with the following arguments:
Dest: LF@main_ln_0
Executing instruction: DEFVAR at line: 8 with the following arguments:
Dest: LF@main_rn_0
Executing instruction: DEFVAR at line: 9 with the following arguments:
Dest: LF@main_ty_0
Executing instruction: DEFVAR at line: 10 with the following arguments:
Dest: LF@main_rs_0
Executing instruction: MOVE at line: 11 with the following arguments:
Arg1: Const@=nil(nil)
Dest: LF@retval()
Executing instruction: PUSHS at line: 14 with the following arguments:
Arg1: Const@=1(int)
Dest: Stack@()
Executing instruction: PUSHS at line: 15 with the following arguments:
Arg1: Const@=1(int)
Dest: Stack@()
Executing instruction: POPS at line: 16 with the following arguments:
Arg1: Stack@=1(int)
Dest: LF@main_rn_0()
Executing instruction: POPS at line: 17 with the following arguments:
Arg1: Stack@=1(int)
Dest: LF@main_ln_0()
Executing instruction: PUSHS at line: 18 with the following arguments:
Arg1: LF@main_ln_0=1(int)
Dest: Stack@()
Executing instruction: PUSHS at line: 19 with the following arguments:
Arg1: LF@main_rn_0=1(int)
Dest: Stack@()
Executing instruction: TYPES at line: 20 with the following arguments:
Arg1: Stack@=1(int)
Dest: Stack@()
Executing instruction: POPS at line: 21 with the following arguments:
Arg1: Stack@=int(string)
Dest: LF@main_ty_0()
Executing instruction: TYPES at line: 22 with the following arguments:
Arg1: Stack@=1(int)
Dest: Stack@()
Executing instruction: PUSHS at line: 23 with the following arguments:
Arg1: LF@main_ty_0=int(string)
Dest: Stack@()
Executing instruction: JUMPIFEQS at line: 24 with the following arguments:
Arg1: Stack@=int(string)
Arg2: Stack@=int(string)
Executing instruction: LABEL at line: 26 with the following arguments:
Executing instruction: PUSHS at line: 27 with the following arguments:
Arg1: LF@main_ty_0=int(string)
Dest: Stack@()
Executing instruction: PUSHS at line: 28 with the following arguments:
Arg1: Const@=string(string)
Dest: Stack@()
Executing instruction: JUMPIFEQS at line: 29 with the following arguments:
Arg1: Stack@=int(string)
Arg2: Stack@=string(string)
Executing instruction: PUSHS at line: 30 with the following arguments:
Arg1: LF@main_ty_0=int(string)
Dest: Stack@()
Executing instruction: PUSHS at line: 31 with the following arguments:
Arg1: Const@=int(string)
Dest: Stack@()
Executing instruction: JUMPIFEQS at line: 32 with the following arguments:
Arg1: Stack@=int(string)
Arg2: Stack@=int(string)
Executing instruction: LABEL at line: 37 with the following arguments:
Executing instruction: PUSHS at line: 38 with the following arguments:
Arg1: LF@main_ln_0=1(int)
Dest: Stack@()
Executing instruction: PUSHS at line: 39 with the following arguments:
Arg1: LF@main_rn_0=1(int)
Dest: Stack@()
Executing instruction: ADDS at line: 40 with the following arguments:
Arg1: Stack@=1(int)
Arg2: Stack@=1(int)
Dest: Stack@()
Executing instruction: JUMP at line: 41 with the following arguments:
Executing instruction: LABEL at line: 48 with the following arguments:
Executing instruction: DEFVAR at line: 50 with the following arguments:
Dest: LF@temp$var$write$0
Executing instruction: POPS at line: 51 with the following arguments:
Arg1: Stack@=2(int)
Dest: LF@temp$var$write$0()
Executing instruction: WRITE at line: 52 with the following arguments:
Arg1: LF@temp$var$write$0=2(int)
Executing instruction: EXIT at line: 55 with the following arguments:
Dest: Const@=0(int)
