# WinGLD
WinGLD, Windows platformunda OpenGL ile grafik geliştirmeyi kolaylaştırmak için tasarlanmış, minimal ve hafif bir C kütüphanesidir.  
Pencere oluşturma, OpenGL bağlama, render döngüsü ve yüksek çözünürlüklü zaman yönetimi gibi temel işleri soyutlar ve hızla grafik uygulamalarına başlamanızı sağlar.

Winapi ile uğraşmadan OpenGL ile ilgilenmek istiyorsanız doğru depodasınız.

## Özellikler
- Windows API ile tam uyumlu, saf C dili ile yazılmış
- Pencere oluşturma ve yönetme (boyutlandırma, pencere barı düzenleme)
- OpenGL 2D/3D render bağlamı oluşturma
- ~60 FPS sabit kare hızı için basit gecikme mekanizması
- Basit ama genişletilebilir mainloop yapısı

## Örnek Kullanım

```c
#include "WGLD/wgld.h"

float color = 0.01f;
int sign = 1;

void loop()
{
    glClearColor(color, 0.17f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    color += 0.01f * sign;
    if (color > 1.0f || color < 0.0f) sign = -sign;
    
    glFlush();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE nop1, LPSTR nop2, int nCmdShow)
{
    WinGL_Device* wgld = WinGL_Init("OpenGL Window", 800, 600, TRUE, hInstance, nCmdShow);
    WinGL_Mainloop(wgld, loop);
    WinGL_Shutdown(wgld);
    return wgld->Windows.Msg.wParam;
}

```
