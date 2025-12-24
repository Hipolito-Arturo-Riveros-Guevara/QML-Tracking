## ✨ Descripción general

La reconstrucción de trayectorias de partículas (**particle tracking**) es una de las tareas fundamentales en los experimentos de Física de Altas Energías (HEP). En detectores modernos (por ejemplo, **ATLAS** y **CMS**) que operan bajo condiciones de **alto pile-up** (HL-LHC), el número de impactos (*hits*) registrados por evento es enorme, lo que convierte la asociación entre hits en un problema **altamente combinatorio**.

Este repositorio contiene la implementación completa de mi proyecto de tesis:

- ✅ **Preprocesamiento de hits de TrackML → construcción de segmentos físicamente plausibles (tripletes)**
- ✅ Reformulación del tracking como un problema de **clasificación binaria de tripletes** (verdaderos vs. falsos)
- ✅ Entrenamiento de **Redes Neuronales Cuánticas (QNN)** y construcción de **Neural Quantum Kernels (NQK)**
- ✅ Uso de **kernels cuánticos + SVM** para la clasificación
- ✅ Evaluación del desempeño mediante **Accuracy**, **Purity** y **Efficiency**
- ✅ Exploración de **PCA** para reducir la dimensionalidad de entrada y la profundidad de los circuitos cuánticos




---<img width="1115" height="567" alt="grafico" src="https://github.com/user-attachments/assets/d5561208-83a6-4683-be2e-9b429f6ed646" />


## 🧠 1) ¿Qué es un Neural Quantum Kernel (NQK)?

Un **Neural Quantum Kernel** combina dos ideas ampliamente utilizadas en *Quantum Machine Learning*:

- **QNNs (Quantum Neural Networks):** circuitos cuánticos parametrizados entrenados con datos
- **Kernels cuánticos explícitos (EQK):** kernels definidos a partir del solapamiento (fidelidad) entre estados cuánticos

**Idea clave:** entrenar primero una QNN pequeña y reutilizar los parámetros aprendidos para definir un **kernel cuántico informado por la tarea**.

En este repositorio nos centramos en la configuración **1-to-3 NQK**:

1. Entrenar una **QNN de 1 qubit** (con *data re-uploading*) → obtener los parámetros óptimos **θ\***
2. Construir un **circuito de embedding** sobre **3 qubits**, replicando el circuito entrenado y añadiendo compuertas de entrelazamiento
3. Calcular la **matriz kernel** **K**
4. Entrenar un **SVM** utilizando **kernels precomputados**



### 2) Particle Tracking

En un experimento de Física de Altas Energías (HEP), una colisión protón–protón produce decenas o cientos de partículas. Al atravesar el detector, las partículas cargadas dejan “marcas” en el sistema de seguimiento (tracker), llamadas **hits**. El problema del **particle tracking** consiste en:

> **Asociar hits dispersos en múltiples capas del detector para reconstruir la trayectoria (track) de cada partícula.**

Esto se complica por tres razones:

1. **Alta densidad de hits (ocupancia)**: en condiciones tipo HL-LHC hay miles de hits por evento.
2. **Pile-up**: muchas colisiones simultáneas generan trayectorias superpuestas.
3. **Combinatoria explosiva**: el número de combinaciones posibles de hits crece rápidamente (e.g., elegir 3 hits de miles ya es enorme).

Por eso, el tracking es uno de los componentes más costosos del pipeline de reconstrucción.

### 3) Formulación del problema

Reconstruir el track completo es un problema global (requiere elegir el conjunto correcto de hits).  
En este trabajo se aborda una versión **local** del problema:

> **Clasificar si un segmento corto de 3 hits consecutivos (un “triplete”) pertenece a la misma partícula.**

Esto es importante porque:
- los tripletes suelen actuar como **semillas (seeds)** para algoritmos de tracking (pattern recognition)
- si clasificamos bien tripletes → reducimos combinatoria aguas abajo → mejor tracking

Formalmente:

- Un **triplete** = 3 hits en capas consecutivas: $(h_1,h_2,h_3)$
- Etiqueta positiva (**1**) si los tres hits comparten el mismo `particle_id`
- Etiqueta negativa (**0**) si combinan hits de partículas distintas

Así, el tracking local se convierte en un **problema de clasificación binaria supervisada**.

---

### 4) Preprocesamiento geométrico

El tracker está inmerso en un campo magnético alineado con el eje del haz.  
Bajo estas condiciones, una partícula cargada sigue una **trayectoria helicoidal**:

- En el plano transversal $x\text{--}y$: curva (aprox. arco de circunferencia)
- En el plano $r\text{--}z$: casi recta, con $r=\sqrt{x^2+y^2}$

Por eso, cada hit $(x,y,z)$ se convierte a:

$$
(r,\phi,z)
$$

y el preprocesamiento usa observables geométricos que “capturan” la física esperada de una trayectoria.

---

### 5) Reducción de combinatoria: dobletes → tripletes con cortes físicos

#### 5.1 Doblettes
Se forman **dobletes**: pares de hits en capas adyacentes.  
Luego se filtran usando dos observables:

- **Intercepto $z_0$** (extrapolación en $r\text{--}z$): aproxima el origen longitudinal
- **Curvatura aparente $\Delta\phi/\Delta r$**: captura variación azimutal por radio

Cortes típicos usados:

- $|z_0|\le 100\ \text{mm}$
- $|\Delta\phi/\Delta r|\le 6\times10^4\ \text{rad/mm}$

Estos cortes eliminan gran parte de los pares no físicos.

#### 5.2 Tripletes
Un **triplete** se construye enlazando dobletes con un hit compartido:

$$
(h_1,h_2) \;\text{y}\; (h_2,h_3) \Rightarrow (h_1,h_2,h_3)
$$

Luego se aplican criterios adicionales:

- **$\theta$-breaking angle**: suavidad en $r\text{--}z$
- **$\phi$-breaking angle**: suavidad en $r\text{--}\phi$
- **estimación de $p_T$**: evita curvaturas compatibles con $p_T$ demasiado bajo

Esto genera un conjunto de tripletes:
- mucho más pequeño que el espacio combinatorio original
- con alta proporción de segmentos plausibles
- listo para entrenar clasificadores

---

### 6) PCA: reducción de dimensionalidad (y por qué ayuda)

Un triplete tiene 9 variables geométricas:

$$
(r_1,\phi_1,z_1,\; r_2,\phi_2,z_2,\; r_3,\phi_3,z_3)\in\mathbb{R}^9
$$

Muchas de estas variables están correlacionadas (por la estructura helicoidal).  
Por ello se explora **PCA**:

- estandariza variables
- encuentra direcciones principales de máxima varianza
- proyecta el dato a un subespacio de dimensión menor (p=3 o p=2)

Esto tiene dos efectos:

✅ reduce redundancia y ruido → mejor generalización  
✅ reduce dimensión de entrada → circuitos cuánticos más cortos y rápidos

---

### 7) Quantum Machine Learning

El núcleo del trabajo es construir un modelo **Neural Quantum Kernel (NQK)**, que combina:

1. **QNN (Quantum Neural Network)**: circuito parametrizado entrenable (1 qubit)  
2. **Quantum kernel explícito**: similitud definida por la fidelidad entre estados embebidos  
3. **SVM** clásico entrenado sobre la matriz kernel precomputada

---

### 8) QNN de 1 qubit con *data re-uploading*

Se utiliza una arquitectura de QNN (1 qubit) en capas. En cada capa:

- se “carga” el dato (rotaciones con $x$)
- se aplican rotaciones entrenables ($\theta$)

Esto permite alta expresividad incluso con 1 qubit, gracias a la repetición de la codificación de datos (*re-uploading*).  
El entrenamiento se realiza minimizando un costo basado en **fidelidad** con estados etiqueta.

Resultado: parámetros óptimos $\theta^\*$ aprendidos del dataset.

---

### 9) De QNN a NQK (1-to-3). Entrenamiento de un Kernel Cuantico con un QNN

Una vez entrenada la QNN de 1 qubit:

- se replica en 3 qubits
- se agregan compuertas de entrelazamiento (CNOTs)
- se define un embedding $S_{\theta^\*}(x)$

El quantum kernel se define como:

$$
k(x,x') = \left|\langle 0|S(x)^\dagger S(x')|0\rangle\right|^2
$$

Esto produce una **matriz kernel** $K_{ij}=k(x_i,x_j)$ que se usa para entrenar un **SVM** con kernel precomputado.

---

### 10) Resultados

Se comparan tres entradas:

- sin PCA (p=9)
- PCA (p=3)
- PCA (p=2)

**Resultado principal:** PCA mejora notablemente el desempeño del NQK, con el mejor caso en p=3:

- p=9: Accuracy 67.23% | Purity 71.36% | Efficiency 75.77%
- p=3: Accuracy 86.69% | Purity 88.40% | Efficiency 86.26%   ✅
- p=2: Accuracy 81.45% | Purity 79.88% | Efficiency 81.10%

Además, purity/efficiency se estudian como función de:
- longitud del track
- $\theta$-breaking angle (suavidad del segmento)

Confirmando que segmentos más suaves se clasifican con mayor estabilidad.

---

### 11) Conclusiones

Este repositorio muestra que:

- El tracking puede abordarse de forma local como clasificación de segmentos.
- Los NQKs son una alternativa viable para aprender una noción de similitud “cuántica” útil.
- La combinación con PCA no solo reduce el costo de simulación cuántica, sino que mejora el rendimiento.

En conjunto, el proyecto ofrece un pipeline reproducible y extensible para explorar QML en tracking HEP.
