<!-- Title -->
<span align = "center">

# Arduino Fuzzy Logic Control Algorithm

Implementation of Fuzzy Logic Control Algorithm written in Arduino

Oleh: Fandi Adinata @2021

</span>
<!-- End of Title -->

<br>

Program fuzzy dalam repository ini menggunakan input berupa selisih antara kecepatan motor dengan setpoint kecepatan motor (error) dan tegangan yang dikonsumsi motor.

## Spesifikasi dan Simulasi
Program dijalankan di perangkat lunak Proteus dengan konfigurasi seperti gambar di bawah

<span align = "center">
   
![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/circuit.png)

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/motor.png)

</span>

## Fuzzy Rule

Rule base menggunakan dua parameter, yaitu nilai error kecepatan motor dan nilai tegangan motor. Motor memiliki kecepatan 160 RPM, sehingga rentang error nya antara -160 dan 160 RPM. Sedangkan tegangan yang digunakan berada pada rentang antara 0V sampai 12V. Keluaran dari rule base berupa nilai PWM dengan rentang 0 – 255 (K = kecil, SD = sedang, B = besar).

<span align = "center">
   
![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_rule.png)

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_rule_graphic.png)

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_segitiga.png)

</span>

## Membership Variable

Fuzzifikasi input error dan tegangan dilakukan dengan menggunakan fungsi segitiga dimana rumus persamaan menjadi seperti dibawah. Wilayah membership dibagi menjadi 5 bagian dengan persamaan masing-masing bagian sebagai berikut:

### Membership Error

<span align = "center">

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_error_membership.png)

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzifikasi_error.png)

</span>

### Membership Tegangan

<span align = "center">

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_voltage_membership.png)

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzifikasi_tegangan.png)

</span>

### Membership Output (PWM)

<span align = "center">

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_output_membership.png)

</span>

## Inference

Dalam pengujian ini, digunakan fungsi implikasi berupa minimum rule sehingga akan dipilih nilai keanggotaan yang lebih kecil untuk setiap rule. Nilai minimum dari setiap rule kemudian digunakan sebagai nilai keanggotaan PWM sesuai dengan rule base.

<span align = "center">

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_rule_base_matlab.png)

</span>

<br>

```
Rule 1 = minimum (NE, MN)		=> K[0]
Rule 2 = minimum (NE, N)		=> K[1]
Rule 3 = minimum (NE, MX)		=> K[2]
Rule 4 = minimum (ZE, MN)		=> SD[0]
Rule 5 = minimum (ZE, N)		=> SD[1]
Rule 6 = minimum (ZE, MX)		=> SD[2]
Rule 7 = minimum (PE, MN)		=> B[0]
Rule 8 = minimum (PE, N)		=> B[1]
Rule 9 = minimum (PE, MX)		=> SD[3]
```

Setelah didapatkan nilai K, SD, dan B PWM dari rule base, maka dipilih nilai paling tinggi dari setiap nilai K, SD, dan B. Sehingga didapatkan nilai K, SD, dan B untuk parameter defuzzifikasi PWM.

```
PWM_K = maksimum (K[0], K[1], K[2])
PWM_SD = maksimum (SD[0], SD[1], SD[2], SD[3])
PWM_B = maksimum (B[0], B[1])
```

## Grafik Output Fuzzy

Dengan fungsi fuzzifikasi segitiga dapat digunakan untuk mencari nilai keanggotaan dari input, maka fungsi ini juga dapat digunakan untuk mendapatkan nilai input dari nilai keanggotaan yang diketahui.

<span align = "center">

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_output_membership.png)
 
Pembentukan grafik output

</span>

```
PWM_K = (C_K – INPUT) / (C_K – B_K)
C_K – INPUT = PWM_K * (C_K – B_K)
INPUT = C_K – (PWM_K * (C_K – B_K)
```
```
Titik potong K = 127,5 – (PWM_K * (127,5 – 0))
Titik potong B = PWM_B * (255 – 127.5) + 127.5
```

Karena grafik SD berada di tengah, maka grafik SD memiliki dua titik potong, yaitu SD_1 dan SD_2.

```
Titik potong SD_1 = PWM_SD * (127,5 – 0) + 0
Titik potong SD_2 = 255 – (PWM_SD * (255 – 127,5))
```

Dengan diketahuinya titik potong, maka dapat dibuat grafik untuk setiap fungsi K, SD, dan B serta grafik PWM akhir. Dengan membagi fungsi PWM menjadi 9 titik (region), maka dapat dibuat grafik yang merepresentasikan hasil akhir rule base. Nilai ini kemudian disimpan dalam bentuk array.

```
Region = {0, 31.875, 63.75, 95.625, 127.5, 159.375, 191.25, 223.125, 255}
```

<span align = "center">

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_graphic_maker.png)

</span>

```
Fungsi K 	=> Bernilai PWM_K apabila <= titik potong K
Fungsi SD 	=> Bernilai PWM_SD apabila diantara SD_1 dan SD_2
Fungsi B	=> Bernilai PWM_B apabila >= titik potong B
```

Kemudian untuk setiap titik sampel dilakukan pemilihan nilai keanggotaan terbersar dari K, SD, dan B yang digunakan untuk membentuk fungsi akhir. Defuzzifikasi dilakukan dengan metode centroid, yaitu menentukan titik tengan dari grafik akhir. Hasil defuzzifikasi ini berupa nilai pwm yang kemudian digunakan untuk kontrol motor.

```
//convert grafik_pwm into pwm value
void defuzzifikasi()
{
  float terbagi = 0;
  float pembagi = 0;
  float centroid = 0;

  // hitung terbagi centroid
  for(int i=0; i<9; i++)
  {
    terbagi = terbagi + (grafik_pwm[i]*fuzzy_region[i]);
  }

  // hitung pembagi centroid
  for(int i=0; i<9; i++)
  {
    pembagi = pembagi + grafik_pwm[i];
  }

  centroid = terbagi / pembagi;

  pwm_out = centroid;

  if (pwm_out > 255)
  {
    pwm_out = 255;
  }

  if(pwm_out < 0)
  {
    pwm_out = 0;
  }
}
```

## Output

Dengan setpoint kecepatan motor di 100 RPM, didapatkan grafik sebagai berikut:

<span align = "center">

![Logo](https://github.com/SuryaAssistant/arduinofuzzy/blob/main/img/fuzzy_output_graphic.png)

</span>
