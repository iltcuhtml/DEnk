section .text
    global main

    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 32


    ; sub
    mov QWORD [rbp - 8], 5
    mov QWORD [rbp - 16], 4
    mov rax, QWORD [rbp - 16]
    sub rax, QWORD [rbp - 8]
    mov QWORD [rbp - 8], rax
    ; /sub


    ; mul
    mov rax, QWORD [rbp - 8]
    mov QWORD [rbp - 16], rax
    mov QWORD [rbp - 24], 4
    mov rax, QWORD [rbp - 24]
    imul QWORD [rbp - 16]
    mov QWORD [rbp - 16], rax
    ; /mul


    ; exit

    ; mul
    mov QWORD [rbp - 24], 5
    mov rax, QWORD [rbp - 16]
    mov QWORD [rbp - 32], rax
    mov rax, QWORD [rbp - 32]
    imul QWORD [rbp - 24]
    mov QWORD [rbp - 24], rax
    ; /mul

    mov rcx, QWORD [rbp - 24]
    call ExitProcess
    ; /exit


    mov rcx, 0
    call ExitProcess