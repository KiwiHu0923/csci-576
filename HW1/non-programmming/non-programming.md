# CS 576 – Assignment 1: Theory Solutions

---

## Question 1 – Bit-Rate and Storage

**Given:** 450 lines/frame, 520 pixels/line, 25 Hz frame rate, 4:2:0 chroma subsampling, interlaced scanning, 8 bits per sample of Y, Cr, Cb.

### Part A: Bit-rate

**Step 1 – Count total samples per frame under 4:2:0:**

$$
Y_{\text{samples}} = 450 \times 520 = 234{,}000
$$

$$
C_r = C_b = \frac{450}{2} \times \frac{520}{2} = 225 \times 260 = 58{,}500 \text{ each}
$$

$$
\text{Total samples/frame} = 234{,}000 + 58{,}500 + 58{,}500 = 351{,}000
$$

**Step 2 – Multiply by bits/sample and frame rate:**

$$
\text{Bit-rate} = 351{,}000 \times 8 \times 25 = 70{,}200{,}000 \text{ bits/sec}
$$

> **Answer: Bit-rate = 70.2 Mbps**

---

### Part B: Disk size for 10 minutes at 6-bit requantization

With 6 bits per channel, the sample count per frame is unchanged (351,000):

$$
\text{Bits/frame} = 351{,}000 \times 6 = 2{,}106{,}000 \text{ bits}
$$

$$
\text{Bit-rate} = 2{,}106{,}000 \times 25 = 52{,}650{,}000 \text{ bits/sec}
$$

$$
\text{Total bits} = 52{,}650{,}000 \times 600 \text{ sec} = 31{,}590{,}000{,}000 \text{ bits}
$$

$$
\text{Total bytes} = \frac{31{,}590{,}000{,}000}{8} = 3{,}948{,}750{,}000 \text{ bytes} \approx 3.95 \text{ GB}
$$

> **Answer: Minimum disk size ≈ 3.95 GB**

---

## Question 2 – Uniform Quantization of Audio Signal

**Setup:** The interval $[-4, 4]$ is divided into 32 uniform levels. Level $k$ has representative value:

$$
q(k) = -3.75 + k \times 0.25, \quad k = 0, 1, \ldots, 31
$$

The step size is $\Delta = 0.25$. A sample $x$ maps to level:

$$
k = \text{round}\!\left(\frac{x + 3.75}{0.25}\right), \quad \text{clamped to } [0, 31]
$$

### Quantized Sequence

| Sample | Level $k$ | Quantized Value |
|-------:|----------:|----------------:|
|   1.8  |    22     |   1.75 |
|   2.2  |    24     |   2.25 |
|   2.2  |    24     |   2.25 |
|   3.2  |    28     |   3.25 |
|   3.3  |    28     |   3.25 |
|   3.3  |    28     |   3.25 |
|   2.5  |    25     |   2.50 |
|   2.8  |    26     |   2.75 |
|   2.8  |    26     |   2.75 |
|   2.8  |    26     |   2.75 |
|   1.5  |    21     |   1.50 |
|   1.0  |    19     |   1.00 |
|   1.2  |    20     |   1.25 |
|   1.2  |    20     |   1.25 |
|   1.8  |    22     |   1.75 |
|   2.2  |    24     |   2.25 |
|   2.2  |    24     |   2.25 |
|   2.2  |    24     |   2.25 |
|   1.9  |    23     |   2.00 |
|   2.3  |    24     |   2.25 |
|   1.2  |    20     |   1.25 |
|   0.2  |    16     |   0.25 |
|  -1.2  |    10     |  -1.25 |
|  -1.2  |    10     |  -1.25 |
|  -1.7  |     8     |  -1.75 |
|  -1.1  |    11     |  -1.00 |
|  -2.2  |     6     |  -2.25 |
|  -1.5  |     9     |  -1.50 |
|  -1.5  |     9     |  -1.50 |
|  -0.7  |    12     |  -0.75 |
|   0.1  |    15     |   0.00 |
|   0.9  |    19     |   1.00 |

### Bits Required

32 levels require $\log_2 32 = 5$ bits per sample.

For 32 samples: $32 \times 5 = 160$ bits total.

> **Answer: 5 bits per sample; 160 bits total for this sequence**

---

## Question 3 – Temporal Aliasing: Rotating Tire

**Given:** car speed $= 36\ \text{km/hr} = 10\ \text{m/s}$, tire diameter $= 0.4244\ \text{m}$, film rate $= 24\ \text{fps}$.

$$
\text{Circumference} = \pi \times 0.4244 \approx 1.3333\ \text{m}
$$

$$
f_{\text{actual}} = \frac{\text{speed}}{\text{circumference}} = \frac{10}{1.3333} = 7.5\ \text{rot/sec}
$$

### Part A: Perceived rate in theater (film at 24 fps)

The Nyquist limit for 24 fps is $f_N = 24/2 = 12\ \text{rot/sec}$.

Since $f_{\text{actual}} = 7.5 < 12$, no aliasing occurs.

> **Answer: Perceived rate = 7.5 rot/sec (same as actual, no aliasing)**

---

### Part B: Camcorder at 8 fps recording the projected movie

The projected movie displays at 24 fps, faithfully showing 7.5 rot/sec. The camcorder samples at 8 fps.

$$
\text{Advance per camcorder frame} = \frac{7.5}{8} = 0.9375\ \text{rotations}
$$

Since $0.9375 > 0.5$ (Nyquist threshold at 8 fps), aliasing occurs:

$$
\text{Apparent advance} = 0.9375 - 1 = -0.0625\ \text{rot/frame (backward)}
$$

$$
\text{Apparent rate} = -0.0625 \times 8 = -0.5\ \text{rot/sec}
$$

> **Answer: Tire appears to rotate backward at 0.5 rot/sec**

---

### Part C: NTSC at 30 fps — maximum speed for no aliasing

No aliasing requires:

$$
f_{\text{rotation}} < f_N = \frac{30}{2} = 15\ \text{rot/sec}
$$

$$
v_{\max} = 15 \times 1.3333 = 20\ \text{m/s} = 72\ \text{km/hr}
$$

> **Answer: Maximum speed = 72 km/hr**

---

## Analysis – Quantization Error vs. Q for Multiple Images

For each image, three curves are plotted: **M = −1** (Uniform), **M = 128** (Logarithmic, pivot = 128), and **M = 256** (Optimal equal-frequency). The error metric is computed with S = 1.0 (no scaling) over Q ∈ {3, 6, 9, 12, 15, 18, 21, 24}.

### MSE Plots

**Lena_512_512**

![Lena MSE](results/Lena_512_512_MSE.png)

**test1_512x512**

![test1 MSE](results/test1_512x512_MSE.png)

**test2_512x512**

![test2 MSE](results/test2_512x512_MSE.png)

**test3_512x512**

![test3 MSE](results/test3_512x512_MSE.png)

**test4_512x512**

![test4 MSE](results/test4_512x512_MSE.png)

**test5_512x512**

![test5 MSE](results/test5_512x512_MSE.png)

### MAE Plots

**Lena_512_512**

![Lena MAE](results/Lena_512_512_MAE.png)

**test1_512x512**

![test1 MAE](results/test1_512x512_MAE.png)

**test2_512x512**

![test2 MAE](results/test2_512x512_MAE.png)

**test3_512x512**

![test3 MAE](results/test3_512x512_MAE.png)

**test4_512x512**

![test4 MAE](results/test4_512x512_MAE.png)

**test5_512x512**

![test5 MAE](results/test5_512x512_MAE.png)

---

### Observations and Trends

#### 1. All curves decrease monotonically as Q increases

Across every image and every mode, both MSE and MAE fall sharply as Q increases from 3 to ~15, then flatten out near zero. This is expected: more bits per pixel → finer quantization intervals → less rounding error. The relationship is approximately exponential in the low-Q regime — each additional 3 bits roughly halves the error.

#### 2. M = −1 (Uniform) and M = 128 (Logarithmic, pivot 128) are nearly identical

In every image, the blue (M = −1) and orange (M = 128) curves are almost perfectly overlapping across all Q values. This is because natural images tend to have a roughly uniform or bell-shaped pixel distribution centered near mid-gray (~128). A logarithmic quantizer pivoted at 128 allocates finer intervals near 128 and coarser intervals near 0 and 255 — but since both tails are roughly equally populated in typical images, the net error is similar to uniform quantization. The two methods only diverge meaningfully when an image is strongly skewed toward dark or bright values.

#### 3. M = 256 (Optimal equal-frequency) outperforms both at low Q, but can underperform at high Q

At Q = 3, 6, and 9 bits, M = 256 consistently achieves significantly lower MSE and MAE than the other two modes — roughly **40–55% lower MSE** at Q = 3 across all images. This is the key advantage of data-driven quantization: by placing interval boundaries so that each bin contains an equal number of pixels (equal-frequency / equi-population binning), the method minimizes the average squared error for the specific pixel distribution of the image at hand.

However, at Q ≥ 18 bits, M = 256 sometimes produces *higher* error than M = −1. This artifact is visible clearly in Lena and test1–test5 at Q = 18, 21, 24. The likely cause is a **floor effect from the rounding/reconstruction step** of the implementation: at very high bit depths, the uniform intervals are already fine enough that reconstruction error is dominated by integer rounding, and the equal-frequency reconstruction values are not always better-positioned than the uniform midpoints.

#### 4. Error magnitude scales with image contrast/complexity

Images with higher dynamic range (test2, test1) show higher absolute MSE at low Q compared to lower-contrast images (Lena, test4). This makes sense: a high-contrast image has pixel values spread across a wider range, so coarse quantization introduces larger errors per pixel. However, the *shape* of the curves (exponential decay, relative ordering of modes) is consistent across all images, confirming these are properties of the quantization methods themselves rather than image-specific artifacts.
