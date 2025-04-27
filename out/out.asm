section .text
    global main

    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 48


    ; mul
    mov QWORD [rbp - 8], 5

    ; div
    mov QWORD [rbp - 16], 2

    ; add
    mov QWORD [rbp - 24], 5
    mov QWORD [rbp - 32], 3
    mov rax, QWORD [rbp - 32]
    add rax, QWORD [rbp - 24]
    mov QWORD [rbp - 24], rax
    ; /add

    mov rax, QWORD [rbp - 24]
    cqo
    idiv QWORD [rbp - 16]
    mov QWORD [rbp - 16], rax
    ; /div

    mov rax, QWORD [rbp - 16]
    imul QWORD [rbp - 8]
    mov QWORD [rbp - 8], rax
    ; /mul


    ; sub

    ; div
    mov QWORD [rbp - 16], 2
    mov QWORD [rbp - 24], 4
    mov rax, QWORD [rbp - 24]
    cqo
    idiv QWORD [rbp - 16]
    mov QWORD [rbp - 16], rax
    ; /div

    mov QWORD [rbp - 24], 7
    mov rax, QWORD [rbp - 24]
    sub rax, QWORD [rbp - 16]
    mov QWORD [rbp - 16], rax
    ; /sub


    ; add
    mov QWORD [rbp - 24], 19

    ; div
    mov QWORD [rbp - 32], 2

    ; mul
    mov rax, QWORD [rbp - 16]
    mov QWORD [rbp - 40], rax
    mov rax, QWORD [rbp - 8]
    mov QWORD [rbp - 48], rax
    mov rax, QWORD [rbp - 48]
    imul QWORD [rbp - 40]
    mov QWORD [rbp - 40], rax
    ; /mul

    mov rax, QWORD [rbp - 40]
    cqo
    idiv QWORD [rbp - 32]
    mov QWORD [rbp - 32], rax
    ; /div

    mov rax, QWORD [rbp - 32]
    add rax, QWORD [rbp - 24]
    mov QWORD [rbp - 24], rax
    ; /add


    ; exit
    mov rax, QWORD [rbp - 24]
    mov QWORD [rbp - 32], rax
    mov rcx, QWORD [rbp - 32]
    call ExitProcess
    ; /exit


    mov rcx, 0
    call ExitProcess