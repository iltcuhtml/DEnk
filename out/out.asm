section .text
    global main

    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    ; Bestimme
    mov QWORD [rbp - 8], 69
    ; /Bestimme

    ; Falls
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
    jnz .L0
    ; /Falls

    ; ExitProcess
    mov QWORD [rbp - 16], 69
    mov rcx, QWORD [rbp - 16]
    call ExitProcess
    ; /ExitProcess

.L0:
    ; Falls
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
    jnz .L1
    ; /Falls

    ; ExitProcess
    mov QWORD [rbp - 16], 420
    mov rcx, QWORD [rbp - 16]
    call ExitProcess
    ; /ExitProcess

.L1:
    ; ExitProcess
    mov QWORD [rbp - 16], 0
    mov rcx, QWORD [rbp - 16]
    call ExitProcess
    ; /ExitProcess
