section .text
    global main

    extern printf
    extern ExitProcess

main:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov rax, 0
    push rax
    mov rax, 1
    push rax
    push QWORD [rsp + 8]

    ; exit
    pop rcx
    call ExitProcess
    ; /exit

    mov rcx, 0
    call ExitProcess