<!-- Title -->
<span align = "center">

# Arduino Fuzzy Logic Control Algorithm

Implementation of Fuzzy Logic Control Algorithm written in Arduino

</span>
<!-- End of Title -->

<br>

Program fuzzy dalam repository ini menggunakan input berupa selisih antara kecepatan motor dengan setpoint kecepatan motor (error) dan tegangan yang dikonsumsi motor.

## Fuzzy Rule

Rule base menggunakan dua parameter, yaitu nilai error kecepatan motor dan nilai tegangan motor. Motor memiliki kecepatan 160 RPM, sehingga rentang error nya antara -160 dan 160 RPM. Sedangkan tegangan yang digunakan berada pada rentang antara 0V sampai 12V. Keluaran dari rule base berupa nilai PWM dengan rentang 0 – 255 (K = kecil, SD = sedang, B = besar).

<span align = "center">

![Logo](https://https://github.com/SuryaAssistant/arduinofuzzy/img/fuzzy_rule.png)

![Logo](https://https://github.com/SuryaAssistant/arduinofuzzy/img/fuzzy_rule_graphic.png)

![Logo](https://https://github.com/SuryaAssistant/arduinofuzzy/img/fuzzy_segitiga.png)

</span>

## Membership Variable
