iota true
newToken tkWord
newToken tkString
newToken tkNumber
newToken tkSymbol
newToken tkBuiltinControl
newToken tkBuiltinStack
newToken tkBuiltinBool
newToken tkBuiltinArith

iota true
newToken tvSymbolBO
newToken tvSymbolBC
newToken tvSymbolPO
newToken tvSymbolPC
newToken tvSymbolCol
newToken tvSymbolSim
newToken tvSymbolDot

iota true
newToken tvControlDot
newToken tvControlCall 
newToken tvControlEval 
newToken tvControlLoop 
newToken tvControlWhile
newToken tvControlGoto 
newToken tvControlIf   
newToken tvControlMacro
newToken tvControlVar
newToken tvControlIncl

iota true
newToken tvBuiltinDrop 
newToken tvBuiltinDup  
newToken tvBuiltinSwap 
newToken tvBuiltinOver 
newToken tvBuiltinRot  
newToken tvBuiltinCRot 
newToken tvBuiltin2Drop
newToken tvBuiltin2Dup 
newToken tvBuiltin2Swap
newToken tvBuiltin2Over
newToken tvBuiltinDropd
newToken tvBuiltinDupd 
newToken tvBuiltinSwapd
newToken tvBuiltinOverd
newToken tvBuiltinRotd 
newToken tvBuiltinNip  
newToken tvBuiltinPick 
newToken tvBuiltinPickd
newToken tvBuiltinDip  

iota true
newToken tvBoolEq
newToken tvBoolLeEq
newToken tvBoolGrEq
newToken tvBoolLe
newToken tvBoolGr

iota true
newToken tvArithAdd
newToken tvArithSub
newToken tvArithMul
newToken tvArithDiv
newToken tvArithMod
newToken tvArithModDiv

iota true
newToken tvNumInt
newToken tvNumFloat
newToken tvNumExp
newToken tvNumFloatExp
; iota true
; newToken subString

struc CodeToken {
  .type db ?
  .subtype db ?
  .line dd ?
  .col dd ?
  .slotw dw ?
  .slotd dd ?
  .slot0 dq ?
  .slot1 dq ?
  .slot2 dq ?
}

virtual at 0
  CodeToken CodeToken
  sizeof.CodeToken = $ - CodeToken
end virtual


struc TokenStorageS {
  .tokens dq ? ; ptr ArrayList
  .count dq ?
}

virtual at 0
  TokenStorageS TokenStorageS
  sizeof.TokenStorageS = $ - TokenStorageS
end virtual


TokenStorage:
.new:
  mov rdi, sizeof.TokenStorageS
  call malloc
  push rax
  call ArrayList.new
  pop rdi
  mov [rdi + TokenStorageS.count], 0
  mov [rdi + TokenStorageS.tokens], rax 
  mov rax, rdi
  ret

.destroy:
  mov rdi, [r15 + TokenStorageS.tokens]
  call free
  mov rdi, r15
  call free
  ret

.add_token:
  inc [r15 + TokenStorageS.count]
  push r15
  mov r15, [r15 + TokenStorageS.tokens]
  call ArrayList.push_back
  pop r15
  ret

.get_token:
  push r15
  mov r15, [r15 + TokenStorageS.tokens]
  call ArrayList.get
  pop r15
  ret

.size:
  mov rax, [r15 + TokenStorageS.count]
  ret

struc LexerS {
  .cursor dq ?
  .string dq ?
  .length dq ?
  .source_id dq ?
  .line dd ?
  .col dd ?
  .output dq ? ; ptr [TokenStorage]
}

virtual at 0
  LexerS LexerS
  sizeof.LexerS = $ - LexerS
end virtual

Lexer:
; input:
;   rdi: source string
;   rsi: source length
;   rdx: source id
;   rcx: output ptr
.new:
  push rdi
  push rsi
  push rdx
  push rcx
  mov rdi, sizeof.LexerS
  call malloc
  pop rcx
  pop rdx
  pop rsi
  pop rdi
  mov [rax + LexerS.cursor], 0
  mov [rax + LexerS.string], rdi
  mov [rax + LexerS.length], rsi
  mov [rax + LexerS.source_id], rdx
  mov [rax + LexerS.line], 1
  mov [rax + LexerS.col], 1
  mov [rax + LexerS.output], rcx
  ret

.destroy:
  mov rdi, r15
  call free
  ret

; output:
;   rax: byte (char)
.current_char:
  push rcx
  push rdx
  mov rcx, [r15 + LexerS.cursor]
  mov rdx, [r15 + LexerS.string]
  xor rax, rax
  mov al, byte [rdx + rcx]
  pop rdx
  pop rcx
  ret


.is_done?:
  push rcx
  push rdx
  mov rax, false
  mov rcx, [r15 + LexerS.cursor] 
  cmp rcx, [r15 + LexerS.length]
  mov rdx, true
  cmove rax, rdx
  pop rdx
  pop rcx
  ret

.is_at_whitespace?:
  push rcx
  push rdx
  call Lexer.current_char
  mov rcx, rax
  mov rax, false
  mov rdx, true
  cmp cl, whitespace
  cmove rax, rdx
  cmp cl, horizontalTab
  cmove rax, rdx
  pop rdx
  pop rcx
  ret

.is_at_newline?:
  push rcx
  push rdx
  call Lexer.current_char
  mov rcx, rax
  mov rax, false
  mov rdx, true
  cmp cl, newline
  cmove rax, rdx
  pop rdx
  pop rcx
  ret

.is_at_string?:
  push rcx
  push rdx
  call Lexer.current_char
  mov rcx, rax
  mov rax, false
  mov rdx, true
  cmp cl, double_quotes
  cmove rax, rdx
  pop rdx
  pop rcx
  ret

.increment_cursor:
  inc [r15 + LexerS.cursor]
  ret

.increment_line:
  inc [r15 + LexerS.line]
  mov [r15 + LexerS.col], 1
  ret

.increment_col:
  inc [r15 + LexerS.col]
  ret

.skip_whitespace:
  .skipping_loop:
    call Lexer.is_done?
    ifTrue Lexer.skipping_loop_end
    call Lexer.is_at_whitespace?
    ifTrue Lexer.skipping_loop_handle_whitespace
    call Lexer.is_at_newline?
    ifTrue Lexer.skipping_loop_handle_newline

    jmp Lexer.skipping_loop_end
    .skipping_loop_handle_whitespace:
      call Lexer.increment_col
      call Lexer.increment_cursor
      jmp Lexer.skipping_loop
    .skipping_loop_handle_newline:
      call Lexer.increment_line
      call Lexer.increment_cursor
      jmp Lexer.skipping_loop
  .skipping_loop_end:
  ret

; output:
;   rax: ptr
;   rdx: len
.move_until_whitespace:
  push r12
  push r13

  mov rax, [r15 + LexerS.cursor]
  mov rdx, [r15 + LexerS.string]
  lea r12, [rdx + rax] ; ptr
  xor r13, r13 ; counter
  .collect_loop:
    call Lexer.is_done?
    ifTrue Lexer.collect_loop_end
    call Lexer.is_at_whitespace?
    ifTrue Lexer.collect_loop_end
    call Lexer.is_at_newline?
    ifTrue Lexer.collect_loop_end
    ; call Lexer.is_at_string? TODO: strings
    ; ifTrue Lexer.collect_string
    
    inc r13
    call Lexer.increment_cursor
    call Lexer.increment_col
    jmp Lexer.collect_loop

  .collect_loop_end:
  mov rax, r12
  mov rdx, r13
  pop r13 
  pop r12
  ret

; output:
;   rax: ptr
;   rdx: len
.next_word:
  call Lexer.skip_whitespace

  call Lexer.is_done?
  cmp rax, true
  mov rax, 0
  je Lexer.next_word_skip

  call Lexer.move_until_whitespace
  .next_word_skip:
  ret

.lex:
  push r12
  push r13
  .lexing:
    call Lexer.next_word
    cmp rax, 0
    je Lexer.lexing_end
    mov r12, rax
    mov r13, rdx 
    
    mov rdi, sizeof.CodeToken
    call malloc
    mov rdx, rax
    push rdx
    push r15
    mov r15, [r15 + LexerS.output]
    mov rdi, rdx
    call TokenStorage.add_token
    pop r15
    pop rdx

    xor rax, rax
    mov eax, [r15 + LexerS.line]
    mov [rdx + CodeToken.line], eax
    mov eax, [r15 + LexerS.col]
    sub rax, r13
    mov [rdx + CodeToken.col], eax

    mov rdi, r12
    mov rsi, r13

    call Lexer.match_symbols_and_keywords
    ifTrue Lexer.lexing_next
    call Lexer.match_number
    ifTrue Lexer.lexing_next

    mov [rdx + CodeToken.type], tkWord
    mov [rdx + CodeToken.subtype], 0

    .lexing_next:
    jmp Lexer.lexing
  .lexing_end:
  pop r13
  pop r12
  ret

macro matchWord name, tk, tks=0 {
  mov rdx, name
  mov rcx, name#.len
  call Memory.cmp
  mov rcx, rax
  xor rax, rax
  xor rdx, rdx
  cmp rcx, true
  mov rcx, tk
  cmove rax, rcx
  mov rcx, tks
  cmove rdx, rcx
  je Lexer.finalize_symbol
}

; input:
;   rdi: ptr string
;   rsi: len string
;   rdx: ptr token
; output:
;   rax: bool
.match_symbols_and_keywords:
  push r12
  mov r12, rdx

  matchWord symBO         , tkSymbol          , tvSymbolBO
  matchWord symBC         , tkSymbol          , tvSymbolBC
  matchWord symPO         , tkSymbol          , tvSymbolPO
  matchWord symPC         , tkSymbol          , tvSymbolPC
  matchWord symCol        , tkSymbol          , tvSymbolCol
  matchWord symSim        , tkSymbol          , tvSymbolSim
  matchWord symDot        , tkSymbol          , tvSymbolDot

  matchWord builtinCall   , tkBuiltinControl  , tvControlCall 
  matchWord builtinEval   , tkBuiltinControl  , tvControlEval 
  matchWord builtinLoop   , tkBuiltinControl  , tvControlLoop 
  matchWord builtinWhile  , tkBuiltinControl  , tvControlWhile
  matchWord builtinGoto   , tkBuiltinControl  , tvControlGoto 
  matchWord builtinIf     , tkBuiltinControl  , tvControlIf  
  matchWord builtinMacro  , tkBuiltinControl  , tvControlMacro
  matchWord builtinVar    , tkBuiltinControl  , tvControlVar
  matchWord builtinIncl   , tkBuiltinControl  , tvControlIncl

  matchWord builtinDrop   , tkBuiltinStack    , tvBuiltinDrop 
  matchWord builtinDup    , tkBuiltinStack    , tvBuiltinDup  
  matchWord builtinSwap   , tkBuiltinStack    , tvBuiltinSwap 
  matchWord builtinOver   , tkBuiltinStack    , tvBuiltinOver 
  matchWord builtinRot    , tkBuiltinStack    , tvBuiltinRot  
  matchWord builtinCRot   , tkBuiltinStack    , tvBuiltinCRot 
  matchWord builtin2Drop  , tkBuiltinStack    , tvBuiltin2Drop
  matchWord builtin2Dup   , tkBuiltinStack    , tvBuiltin2Dup 
  matchWord builtin2Swap  , tkBuiltinStack    , tvBuiltin2Swap
  matchWord builtin2Over  , tkBuiltinStack    , tvBuiltin2Over
  matchWord builtinDropd  , tkBuiltinStack    , tvBuiltinDropd
  matchWord builtinDupd   , tkBuiltinStack    , tvBuiltinDupd 
  matchWord builtinSwapd  , tkBuiltinStack    , tvBuiltinSwapd
  matchWord builtinOverd  , tkBuiltinStack    , tvBuiltinOverd
  matchWord builtinRotd   , tkBuiltinStack    , tvBuiltinRotd 
  matchWord builtinNip    , tkBuiltinStack    , tvBuiltinNip  
  matchWord builtinPick   , tkBuiltinStack    , tvBuiltinPick 
  matchWord builtinPickd  , tkBuiltinStack    , tvBuiltinPickd
  matchWord builtinDip    , tkBuiltinStack    , tvBuiltinDip  

  matchWord builtinEq     , tkBuiltinBool     , tvBoolEq
  matchWord builtinLeEq   , tkBuiltinBool     , tvBoolLeEq
  matchWord builtinGrEq   , tkBuiltinBool     , tvBoolGrEq
  matchWord builtinLe     , tkBuiltinBool     , tvBoolLe
  matchWord builtinGr     , tkBuiltinBool     , tvBoolGr

  matchWord builtinAdd    , tkBuiltinArith    , tvArithAdd
  matchWord builtinSub    , tkBuiltinArith    , tvArithSub
  matchWord builtinMul    , tkBuiltinArith    , tvArithMul
  matchWord builtinDiv    , tkBuiltinArith    , tvArithDiv
  matchWord builtinMod    , tkBuiltinArith    , tvArithMod
  matchWord builtinModDiv , tkBuiltinArith    , tvArithModDiv

  mov rax, false
  mov rdx, r12
  pop r12
  ret 
  .finalize_symbol:
  mov [r12 + CodeToken.type], al
  mov [r12 + CodeToken.subtype], dl
  
  mov rax, true
  mov rdx, r12
  pop r12
  ret


; input:
;   rdi: character
.is_number?:
  mov rax, true
  cmp dil, 48
  jl Lexer.is_number_false
  cmp dil, 57
  jg Lexer.is_number_false
  jmp Lexer.is_number_true
  .is_number_false:
  mov rax, false
  .is_number_true:
  ret

; input:
;   rdi: character
.is_number_middle?:
  call Lexer.is_number?
  ifTrue Lexer.is_number_middle_true
  cmp dil, 95
  je Lexer.is_number_middle_true
  mov rax, false
  .is_number_middle_true:
  ret

; input:
;   rdi: ptr
;   rsi: len
;   rdx: ptr token
; output:
;   rax: bool
.match_number:
  push rbx
  push rdi
  push r12
  mov r12, rdi
  xor rdi, rdi
  mov dil, byte [r12]
  call Lexer.is_number?
  ifFalse Lexer.match_number_no_match
  mov rbx, 1
  .match_number_loop:
    cmp rbx, rsi
    je Lexer.match_number_match
    
    mov dil, byte [r12 + rbx]
    call Lexer.is_number_middle?
    ifFalse Lexer.match_number_no_match
    inc rbx

  .match_number_match:
    mov [rdx + CodeToken.type], tkNumber
    mov [rdx + CodeToken.subtype], tvNumInt
    mov rax, true
  .match_number_no_match:
  pop r12
  pop rdi
  pop rbx
  ret