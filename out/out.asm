section .text
    global main

    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    mov QWORD [rbp - 8], 0

    ; if

    ; sub
    mov QWORD [rbp - 16], 69
    mov rax, QWORD [rbp - 8]
    mov QWORD [rbp - 24], rax
    mov rax, QWORD [rbp - 24]
    sub rax, QWORD [rbp - 16]
    mov QWORD [rbp - 16], rax
    ; /sub

    mov rax, QWORD [rbp - 16]
    test rax, rax
    jz .L0

    ; exit
    mov QWORD [rbp - 16], 69
    mov rcx, QWORD [rbp - 16]
    call ExitProcess
    ; /exit

    ; /if

.L0:

    ; if

    ; sub
    mov QWORD [rbp - 16], 420
    mov rax, QWORD [rbp - 8]
    mov QWORD [rbp - 24], rax
    mov rax, QWORD [rbp - 24]
    sub rax, QWORD [rbp - 16]
    mov QWORD [rbp - 16], rax
    ; /sub

    mov rax, QWORD [rbp - 16]
    test rax, rax
    jz .L1

    ; exit
    mov QWORD [rbp - 16], 420
    mov rcx, QWORD [rbp - 16]
    call ExitProcess
    ; /exit

    ; /if

.L1:

    ; exit
    mov QWORD [rbp - 16], 0
    mov rcx, QWORD [rbp - 16]
    call ExitProcess
    ; /exit

