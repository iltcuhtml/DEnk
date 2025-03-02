section .data
    message db "WORLD!", 0

section .text
    global main

    extern printf
    extern ExitProcess

main:
    sub rsp, 40

    lea rcx, [rel message]
    call printf

    mov rcx, 0
    call ExitProcess