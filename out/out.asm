section .text
    global main

    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    ; Bestimme
    mov QWORD [rbp - 8], 69
    ; /Bestimme

    ; ExitProcess
    mov rax, QWORD [rbp - 8]
    mov QWORD [rbp - 16], rax
    mov rcx, QWORD [rbp - 16]
    call ExitProcess
    ; /ExitProcess
