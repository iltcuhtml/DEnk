section .text
    global main

    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 48


    ; add
    mov QWORD [rbp - 8], 9

    ; add

    ; div
    mov QWORD [rbp - 16], 6

    ; mul
    mov QWORD [rbp - 24], 24
    mov QWORD [rbp - 32], 5
    mov rax, QWORD [rbp - 32]
    imul QWORD [rbp - 24]
    mov QWORD [rbp - 24], rax
    ; /mul

    mov rax, QWORD [rbp - 24]
    cqo
    idiv QWORD [rbp - 16]
    mov QWORD [rbp - 16], rax
    ; /div

    mov QWORD [rbp - 24], 3
    mov rax, QWORD [rbp - 24]
    add rax, QWORD [rbp - 16]
    mov QWORD [rbp - 16], rax
    ; /add

    mov rax, QWORD [rbp - 16]
    add rax, QWORD [rbp - 8]
    mov QWORD [rbp - 8], rax
    ; /add


    ; sub
    mov QWORD [rbp - 16], 2
    mov QWORD [rbp - 24], 7
    mov rax, QWORD [rbp - 24]
    sub rax, QWORD [rbp - 16]
    mov QWORD [rbp - 16], rax
    ; /sub


    ; sub
    mov QWORD [rbp - 24], 11

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
    sub rax, QWORD [rbp - 24]
    mov QWORD [rbp - 24], rax
    ; /sub


    ; exit
    mov rax, QWORD [rbp - 24]
    mov QWORD [rbp - 32], rax
    mov rcx, QWORD [rbp - 32]
    call ExitProcess
    ; /exit

    mov rcx, 0
    call ExitProcess