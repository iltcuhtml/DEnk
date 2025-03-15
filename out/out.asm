section .text
    global main

    extern printf
    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    mov QWORD [rbp - 8], 69
    mov QWORD [rbp - 16], 74
    mov rax, QWORD [rbp - 8]
    mov QWORD [rbp - 24], rax
    mov rax, QWORD [rbp - 24]
    mov QWORD [rbp - 32], rax

    ; exit
    mov rax, QWORD [rbp - 32]
    mov rcx, rax
    call ExitProcess
    ; /exit

    mov rcx, 0
    call ExitProcess