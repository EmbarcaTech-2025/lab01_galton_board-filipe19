# **EmbarcaTech HBr 2025**  

**Institution:** Instituto Hardware BR-DF  
**Course:** Technological Residency in Embedded Systems  
**Author:** Filipe Alves de Sousa  
**Brasília-DF, May 2025**  

# **Digital Galton Board**  

## Configurando o Problema
Em um tabuleiro Galton idealizado, como o mostrado na Figura 1, uma bola quica contra um único pino em cada fileira do tabuleiro. Quando a bola atinge o pino, ela tem 50% de probabilidade de quicar para a esquerda e 50% de probabilidade de quicar para a direita. Nessa idealização, o resultado do quique em cada fileira é completamente independente dos quiques nas fileiras anteriores.

![Image](https://github.com/user-attachments/assets/8b778056-55c2-4cd2-a578-e20ec5abdc4d)
*Figura 1. Visão geral do Conselho Galton (Fonte: [V. Hunter Adams](https://vanhunteradams.com/Pico/Galton/Galton.html#Introduction))*

Um conjunto de receptáculos captura as bolas conforme elas caem pela fileira inferior do tabuleiro. Podemos rotular cada uma desses receptáculos começando pela esquerda com números inteiros crescentes a partir de 0. O número de caixas será igual a um a mais que o número de linhas em nosso Tabuleiro de Galton.
A bola cai no pino central superior, em uma posição que é metade do número de linhas no tabuleiro. Para o Tabuleiro de Galton de 5 linhas mostrado acima, por exemplo, a bola cai no tabuleiro na posição 2,5. Cada impacto no pino tem 50% de probabilidade de aumentar sua posição em 0.5 (a bola cai para a direita) e uma probabilidade de 50% de diminuir sua posição em 0.5 (a bola cai para a esquerda). A caixa na qual ela finalmente cai representa a soma de N tais ensaios aleatórios, onde N é o número de linhas no tabuleiro.


## O que é a Placa de Galton?
É um dispositivo físico que demonstra como pequenas decisões aleatórias levam a uma distribuição normal (Gaussiana) quando repetidas muitas vezes (como a decisão aleatória de ir para esquerda ou direita após uma colisão).


**Por que digital?**  
Transformei esse conceito em um simulador interativo, onde podemos ajustar parâmetros em tempo real e ver os resultados instantaneamente.

![Image](https://github.com/user-attachments/assets/7b051d18-c318-43bf-a24c-10185dbc472d)
_Foto do display oled durante os testes - developmente environment._  

---


Este projeto implementa uma versão digital da **Galton Board** utilizando a Raspberry Pi Pico com o kit **BitDogLab**. A simulação permite visualizar em tempo real o comportamento estatístico de partículas que colidem com pinos fixos e se acumulam em "bins", ilustrando conceitos como **distribuição binomial**, **Teorema Central do Limite** e a **Lei dos Grandes Números**.


---

## Componentes e Arquitetura

** Hardware Utilizado:**
- Microcontrolador: Raspberry Pi Pico (ARM Cortex-M0+) arquitetura baseada no RP2040
- Display: OLED 128x64 (I2C)
- Controles: 2 botões (GPIO 5, 6)

![Image](https://github.com/user-attachments/assets/f7c703dd-2d34-49cb-bfd7-568ae25165a4)
_Foto da BitDogLab e ao fundo, o VSCode developmente environment._  


** Arquitetura do Código (C):**
1. **Inicialização:** Configura GPIO, I2C e display
2. **Simulação Física:** Atualiza posição das bolinhas e colisões
3. **Renderização:** Desenha tudo no OLED pixel a pixel utilizando a biblioteca disponível na pasta inc.
4. **Controles:** Botões A e B, que ajustam parâmetros em tempo real


![Image](https://github.com/user-attachments/assets/36775d98-e399-4e9b-bb96-c9202b3d1a4a)
_Photo of my implementation in the VSCode developmente environment._  

---

## Funcionamento Prático

### Aleatoriedade e Colisões
- Cada bolinha começa no centro da canaleta superior
- Ao colidir com um pino, ela tem probabilidade ajustável de ir para esquerda ou direita
- Viés ajustável via botão B (0 = sempre esquerda, 5 = 50%, 10 = sempre direita)

### Contagem e Distribuição
- As bolinhas caem nos receptáculos inferiores, formando um histograma
- Normalização automática: O gráfico se ajusta para caber no display

### Múltiplas Bolinhas Simultâneas
- Botão A controla número de bolinhas liberadas por ciclo (1 a 5)
- Demonstração prática da Lei dos Grandes Números

---

## Por que esse projeto é especial?

 **União de teoria e prática:**  
Combina conceitos de probabilidade com implementação em sistemas embarcados

 **Desafio técnico:**  
Todo o processamento gráfico é feito pixel a pixel no microcontrolador

 **Interatividade:**  
Controle em tempo real dos parâmetros da simulação

---

## Materiais e Conexões

| Componente            | Conexão no BitDogLab     |
|-----------------------|--------------------------|
| BitDogLab (RP2040)    | Raspberry Pi Pico W      |
| OLED Display I2C      | SDA: GPIO14 / SCL: GPIO15|
| Button A              | GPIO5                    |
| Button B              | GPIO6                    |

---

## Estrutura do Projeto

```bash
lab01_galton_board-filipe19/
├── inc/
│   ├── ssd1306.h           # Biblioteca para controle do display
│   ├── ssd1306_i2c.[ch]    # Driver I2C para o display
├── src/
│   ├── galton_config.h     # Configurações e constantes
│   ├── galton_display.c    # Renderização e inicialização
│   └── galton_simulation.c # Lógica da simulação
├── assets/                 # Imagens e GIFs demonstrativos
├── CMakeLists.txt          # Configuração de compilação
└── README.md               # Documentação
```

---

## Controles Interativos

| Botão | Função | Valores |
|-------|--------|---------|
| A (GP5) | Bolas por ciclo | 1-5 (exibido como "A:X") |
| B (GP6) | Desbalanceamento | 0-10 (0=esquerda, 5=balanceado, 10=direita) |

**Display mostra:**
- Total de bolas ("T:XXXX")
- Histograma normalizado
- Elementos visuais (pinos, trajetórias, canaleta, receptáculos)

---

## Princípios Matemáticos

```c
// Exemplo: Decisão com viés
bool random_decision_with_bias() {
    int threshold = map(BALANCE_BIAS, 0, 10, 5, 95);
    return (rand() % 100) < threshold;
}
```

**Conceitos demonstrados:**
- Distribuição Binomial
- Teorema Central do Limite
- Lei dos Grandes Números
- Processos Estocásticos

---

## Parâmetros Ajustáveis

| Parâmetro | Valores | Efeito |
|-----------|---------|--------|
| GRAVITY | 0.1-0.5 | Velocidade de queda |
| BOUNCINESS | 0.1-0.9 | Elasticidade nas colisões |
| PIN_SPACING | 5-15 | Espaçamento entre pinos |
| MAX_HISTOGRAM_HEIGHT | 10-20 | Altura máxima do gráfico |

---

## Demonstração em Vídeo

[![Assista ao vídeo](https://img.youtube.com/vi/KARrL2JzPGs/0.jpg)](https://www.youtube.com/watch?v=KARrL2JzPGs)

---

## 🔧 Como Compilar

```bash
mkdir build && cd build
cmake ..
make
```

Carregue o arquivo `.uf2` gerado na Pico.

---

## *Controle interativo:* 
Botão A (GP5):

Cicla entre 1-5 bolas liberadas simultaneamente

Exibido como "A:X" no canto superior esquerdo

Botão B (GP6):

Ajusta o desbalanceamento (0-10)

0 = Viés máximo para esquerda

5 = Distribuição balanceada (padrão)

10 = Viés máximo para direita

Exibido como "B:X" no canto superior direito


## Exibição de Dados:
*Total de bolas:* "T:XXXX" no canto inferior esquerdo

*Histograma:*
Normalizado automaticamente
Altura máxima configurável (MAX_HISTOGRAM_HEIGHT)

*Elementos visuais:*
Pinos hexagonais
Trajetórias das bolas
Canaletas / receptáculos

---


### 1. Contador Total de Bolas

Abaixo, o contador no canto da tela aumenta conforme novas bolas são geradas e caem nos bins:


![Contador Total de Bolas](https://github.com/user-attachments/assets/89c5a211-af18-403d-8df4-4433f3f387c8)

_Photo of my implementation - Contador Total de Bolinhas._  


---

### 2. Simulação com Desbalanceamento

Aqui, o botão foi usado para aplicar viés à direita. Note a assimetria da distribuição final:

![Simulação com Viés](https://github.com/user-attachments/assets/5a47eb1e-7c77-463e-b45d-708d8b0dd126)


_Photo of my implementation - Simulação com Viés - tendência para esquerda/direita._  

---

### 3. Múltiplas Bolas Simultâneas

Nesta simulação, várias partículas caem ao mesmo tempo, aumentando a velocidade da formação do histograma:

![Várias Bolas ao Mesmo Tempo](https://github.com/user-attachments/assets/c9208a5b-c449-4274-82e7-0fdb2ce7744b)

_Photo of my implementation - Várias Bolas ao Mesmo Tempo._  


## Requisitos

* Raspberry Pi Pico com BitDogLab ou similar
* Display OLED SSD1306 (via I2C)
* Botões
* Ambiente de desenvolvimento para C/C++ com SDK da Pico


## Referências

1. Adams, V. H. "Digital Galton Board". Disponível em: [https://vanhunteradams.com/Pico/Galton/Galton.html](https://vanhunteradams.com/Pico/Galton/Galton.html)
2. Raspberry Pi Pico C/C++ SDK Documentation
3. "The Galton Board: 150 Years of Normal Distribution" - Statistical Science
4. Princípios de Simulação Física - David H. Eberly

---

## Licença

Este projeto está licenciado sob a [MIT License](LICENSE).
