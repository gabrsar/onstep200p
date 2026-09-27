# OnStep200P

Controlador Alt-Az para ESP32-S3 baseado no OnStepX, com OLED SSD1306, joystick e som. Nasceu em um Sky-Watcher200P e está em desenvolvimento.

**Motores, redução e tracking ainda não foram validados.** Interface, calibração do joystick e sincronização Stellarium foram testadas em bancada. Valores provisórios não são configuração pronta para seu telescópio.

```sh
git clone --recurse-submodules https://github.com/gabrsar/onstep200p.git
cd onstep200p
make setup
make quality
make check
```

`make check` testa/compila; `make install PORT=...` grava. Veja [build](BUILD.md), [hardware](HARDWARE.md), [controles](CONTROLS.md) e [protocolo](PROTOCOL.md).

Joystick: VRx→GPIO5, VRy→GPIO4, SW→GPIO6 e3,3V. ALT: EN10/STEP9/DIR8. Speaker usa circuito de acionamento, nunca carga direta no GPIO7.

Use o IP do OLED e porta9999. AP de desenvolvimento: OnStep200P, senha pública `onstepx200p`. Credenciais pessoais ficam somente em `config/Wifi.local.h`; o binário local também pode contê-las. Após reboot, sincronize data/hora/localização pelo app.

Previews são simulados e usam dados fictícios. Veja [roadmap](ROADMAP.md) e [créditos](../NOTICE.md). Licença GPL-3.0.
