section .text
    global main

    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov QWORD [rbp - 8], 0

    ; if
    mov rax, QWORD [rbp - 8]
    mov QWORD [rbp - 16], rax
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

    ; exit
    mov QWORD [rbp - 16], 0
    mov rcx, QWORD [rbp - 16]
    call ExitProcess
    ; /exit

