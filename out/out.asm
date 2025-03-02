section .text
    global main

    extern printf
    extern ExitProcess

main:
    sub rsp, 40

    mov rcx, 0
    call ExitProcess