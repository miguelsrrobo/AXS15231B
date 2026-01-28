## 📺 JC3248W535EN – AXS15231B + LVGL (ESP-IDF)

Este repositório contém uma **versão baseada em ESP-IDF**, utilizando o **display com controlador AXS15231B** integrado ao **LVGL**.

![JC3248W535EN Board](docs/IMG_6782.jpg)

---

### 🔧 Tecnologias utilizadas

Internamente, o projeto é baseado em:

* **ESP-IDF 5.3**
* **LVGL 8.x**

O uso do **ESP-IDF 5.3** é necessário porque o driver do display depende diretamente de recursos presentes apenas nessa versão do framework.

---

![Display em funcionamento](docs/IMG_6781.jpg)

---

### 🎯 Objetivo do projeto

Atualmente, o projeto implementa a luz de fundo a ideia e implmentar com forma de estudo uma **interface gráfica simples**, contendo:

* Uma tela principal
* Um botão central
* Um contador que registra a quantidade de vezes que o botão foi pressionado

O principal objetivo deste repositório é **estudar e validar o funcionamento do display**, bem como **explorar o uso do framework LVGL 8.x** e a implementação de drivers de forma manual em conjunto com o ESP-IDF.

![Exemplo da interface gráfica](docs/IMG_6783.jpg)

---

### 🧩 Hardware

Este projeto foi desenvolvido para a placa:

**JC3248W535EN – AXS15231B**

Disponível em:
🔗 [https://s.click.aliexpress.com/e/_DFO5uIV](https://s.click.aliexpress.com/e/_DFO5uIV)

