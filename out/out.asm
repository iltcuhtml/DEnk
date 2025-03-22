section .text
    global main

    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 64

    mov QWORD [rbp - 8], 6900
    mov QWORD [rbp - 16], 69
    mov rax, QWORD [rbp - 8]
    mov QWORD [rbp - 24], rax
    mov rax, QWORD [rbp - 16]
    mov QWORD [rbp - 32], rax
    mov QWORD [rbp - 40], 5
    mov rdx, QWORD [rbp - 32]
    mov rax, QWORD [rbp - 40]
    add rax, rdx
    mov QWORD [rbp - 32], rax
    mov rdx, QWORD [rbp - 24]
    mov rax, QWORD [rbp - 32]
    add rax, rdx
    mov QWORD [rbp - 24], rax
    mov rax, QWORD [rbp - 24]
    mov QWORD [rbp - 32], rax

    ; exit
    mov rcx, QWORD [rbp - 32]
    call ExitProcess
    ; /exit

    mov rcx, 0
    call ExitProcess