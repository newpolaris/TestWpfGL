> https://github.com/MonoMaxMW/MonoMax.GLPlayground

위 프로젝트 기반을 우선 테스트,
resize 시 흰색으로 표시되는 문제 수정

> https://github.com/dwmkerr/sharpgl 

fastGL은 c# d3dimage 에 OpenGL 확장을 이용하여, opengl 이미지 표시
HiddenWindow는 pixel owrndership 으로 인해 Form 기반에 동작
FBO는 glReadpixel 의 data를 HBITMAP 로 변경, image의 source로 만드는데,
hbitmap을 변경하는 과정에서 glReadPixel 만큼의 시간이 소모된다
interface 유지를 위해, 해당 방식을 택한거 같긴한데
2560x1600 이었나, 풀 화면 렌더링시 15~20 millsec 가 걸린다
