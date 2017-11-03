# myDrivers 

Wellcom to my myDrivers!
==================

개발 환경

- H/W : Zynq Board
- S/W : VMware(ubuntu 14.04)
- Cross-Compilation : buildroot-2017.08


해당 파일들을 다운받고 컴파일 하기 위해서 buildroot-2017.08/output/build/xilinx-linux..../driver

폴더에 몇가지 파일들을 고쳐야 한다. 내용은 아래 사이트를 참고하시오.

[커널모듈 추가하기](https://www.pcw1029.com/single-post/2017/07/28/Buildroot-%EC%BB%A4%EB%84%90-%EB%AA%A8%EB%93%88-%EC%B6%94%EA%B0%80)


chMemMap
--------

Platform device driver를 기반으로 만들어 졌으며 character형 디바이스 드라이버이다.

징크에서 PL과 PS간의 AXI-Interface로 데이터 교환을 하고, 메모리 크기만큼 리눅스에서 

접근하여 데이터를 읽고 쓸수 있다.

> **Note:**
> - Device Tree의 compatible이름을 맞춰줘야 한다.

