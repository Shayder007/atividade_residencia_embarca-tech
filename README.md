# Projetos com RP2040 - Embarca Tech

Este repositório contém os códigos desenvolvidos durante as atividades práticas da **Residência Tecnológica Embarca Tech**, utilizando o microcontrolador **RP2040** (Raspberry Pi Pico / BitDogLab) com sensores e atuadores.

---

## Como testar os códigos

### Pré-requisitos

- Placa: Raspberry Pi Pico / Pico W / BitDogLab
- SDK da Raspberry Pi Pico instalado corretamente (C/C++)
- VS Code com extensões do CMake configuradas
- Cabos, jumpers e os sensores usados no projeto
- Serial monitor habilitado (pode ser via VS Code, PuTTY, Thonny, etc.)

---

### Executando um exemplo

1. **Abra a pasta do projeto** desejado no VS Code 
2. **Conecte sua placa** via USB
3. Pressione o botão BOOTSEL e plugue o cabo
4. Compile e envie o código:
   - Clique em "Build"
   - Depois clique em "Upload" (ou use `Ctrl+Shift+B`)
5. **Abra o monitor serial** e veja os dados em tempo real
---

### Dicas para testes

- **BH1750 (luminosidade)**: Use luz natural, lanterna ou abafe com a mão.
- **MPU6050 (movimento)**: Incline suavemente a placa nas direções X e Y.
- **Servo Motor SG90**: Verifique a alimentação correta (3.3V ou fonte externa de 5V) e sinal PWM no pino indicado no código.

---

### Observações

- Os códigos estão comentados para facilitar modificações.
- Cada projeto foi testado em uma BitDogLab, mas também funcionam em outras placas com RP2040.
- Em caso de erro com dependências, reinstale o SDK e verifique o `CMakeLists.txt`.

---

### Autor

**Shayder Faustino do Nascimento**  
Residência Embarca Tech – IFPI / 2025  
Projeto prático com sensores e atuadores no RP2040

---
