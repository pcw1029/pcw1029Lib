# myApplications

Wellcom to my Applications!
==========================

개발 환경

- H/W : Zynq Board
- S/W : VMware(ubuntu 14.04)
- Cross-Compilation : buildroot-2017.08


buildroot의 package를 추가하는 내용은 아래 사이트를 참고하시오.

[PACKAGE 추가하기](https://www.pcw1029.com/single-post/2017/10/24/Buildroot-Package-추가)


tcpIp
--------

기본적인 에코 서버이다.

쓰레드가 총 3개(send, recv, command)로 구현되어있으며, 클라이언트가 하나 접속하게 되면

큐가 생성되고, recv에서 받은 데이터는 recvQueue에 저장하고 command 쓰레드가 큐에 쌓여있는

데이터를 꺼내 어떤 처리(메시지 복사)를 수행하며 수행 결과를 sendQueue로 보내게 된다.

send쓰레드에서는 sendQueue가 쌓여있으면 클라이언트로 데이터를 전송한다.

> **Note:**
> - 일단 클라이언트에 포트 번호를 8899로 하드코딩되어있으므로 서버 실행시 ./server 8899로 수행하면 된다.

