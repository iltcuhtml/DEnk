section .data
    LC0 db "WORLD!", 0  ; 문자열 리터럴 (null-terminated)

section .text
    global main

    extern printf
    extern ExitProcess  ; Windows API ExitProcess 사용

main:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov QWORD [rbp - 8], 0  ; 변수 초기화
    mov rax, QWORD [rbp - 8]
    mov QWORD [rbp - 16], rax

    mov rdi, LC0   ; printf("%s", "WORLD!");
    xor eax, eax   ; 호출 전 eax를 0으로 설정 (64비트 호출 규칙)
    call printf

    mov rax, QWORD [rbp - 16]  ; ExitProcess(0)
    mov ecx, eax               ; Windows API 호출 규칙: exit code는 rcx에 저장
    call ExitProcess