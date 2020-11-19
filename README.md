> https://github.com/MonoMaxMW/MonoMax.GLPlayground

위 프로젝트 기반을 우선 테스트,
resize 시 흰색으로 표시되는 문제 수정

> https://github.com/dwmkerr/sharpgl 

fastGL은 FastGLControl에서 D3DImage에 Lock, AddDirtyRect, Unlock 호출

HiddenWindow는 pixel owrndership 으로 인해 Form 기반에 동작
FBO는 glReadpixel 의 data를 HBITMAP 로 변경, image의 source로 만드는데,
hbitmap을 변경하는 과정에서 glReadPixel 만큼의 시간이 소모된다
interface 유지를 위해, 해당 방식을 택한거 같긴한데
2560x1600 이었나, 풀 화면 렌더링시 15~20 millsec 가 걸린다

> https://github.com/microsoft/WPFDXInterop

돋보기 구현 샘플 제공, 
D3DImage를 상속받아서, D3D11Image 를 구성함

> https://archive.codeplex.com/?p=sharpdxwpf

D3DImage를 상속받아서, DXImageSource 를 구성함

> https://docs.microsoft.com/ko-kr/dotnet/desktop/wpf/advanced/walkthrough-hosting-direct3d9-content-in-wpf?view=netframeworkdesktop-4.8

D3DImage 사용 예제, directx 구현은 안보여줌
