#define pwm_pin 9
#define counter_pin 2

int counter = 0;

unsigned long millis_time;
unsigned long prev_millis = 0;
unsigned long millis_interval = 100;

float standard_pulse_per_revolution = 100.0;
float ppr = 0;
float rps = 0;
float rpm = 0;

float bat_teg = 0;
float mos_teg = 0;
float motor_teg = 0;

int pwm_out = 0;;
float error = 0.0;
float prev_error = 0;
float set_speed = 100;

// Fuzzy Components
float error_neg;
float error_zero;
float error_pos;
float power_min;
float power_normal;
float power_max;
float pwm_k;
float pwm_sd;
float pwm_b;

float grafik_pwm[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
float fuzzy_region[9] = {0, 31.875, 63.75, 95.625, 127.5, 159.375, 191.25, 223.125, 225};


void setup() {
  // put your setup code here, to run once:
  pinMode(pwm_pin, OUTPUT);
  pinMode(counter_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(counter_pin), encoder_read, RISING);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  
  Serial.begin(9600);

  Serial.println("CLEARDATA");
  Serial.println("LABEL,Computer Time,Time(millis),RPM,Volt");
}

void loop() {
  // put your main code here, to run repeatedly:
  millis_time = millis();
  analogWrite(pwm_pin, pwm_out);

  // read voltage accross motor
  bat_teg = analogRead(A0)/1024.0 * 5.0 * 3.0;
  mos_teg = analogRead(A1)/1024.0 * 5.0 * 3.0;
  motor_teg = bat_teg - mos_teg;

  if (millis_time - prev_millis >= millis_interval)
  {
    // convert counter to revolution
    ppr = counter / standard_pulse_per_revolution;

    // reset counter
    counter = 0;

    // convert ppr to rotation per second
    rps = ppr * (1000.0 / millis_interval);

    // convert rps to rotation per minutes
    rpm = rps * 60.0;


    // Print the number
    Serial.print("DATA,TIME,");
    Serial.print(millis_time/1000.0);
    Serial.print(",");
    Serial.print(rpm);
    Serial.print(",");
    Serial.println(motor_teg);

    // PID
    // get error value;
    error = set_speed - rpm;

    fuzzifikasi_error();
    fuzzifikasi_power();
    fuzzy_rule();
    defuzzifikasi();
    
    prev_error = error;
    prev_millis = millis_time;


    
  }
  
}

void encoder_read()
{
  counter ++;
}

void fuzzifikasi_error()
{
  
  if(error <= -160)
  {
    error_neg = 1;
    error_zero = 0;
    error_pos = 0;
  }
  if(error > -160 && error < 0)
  {
    error_neg = (0 - error)/(0 - (-160));
    error_zero = (error - (-160)) / (0 - (-160));
    error_pos = 0;
  }
  if(error == 0)
  {
    error_neg = 0;
    error_zero = 1;
    error_pos = 0;
  }
  if(error > 0 && error < 160)
  {
    error_neg = 0;
    error_zero = (160 - error)/(160 - 0);
    error_pos = (error - 0) / (160 - 0);
  }
  if(error == 160)
  {
    error_neg = 0;
    error_zero = 0;
    error_pos = 1;
  }
}

void fuzzifikasi_power()
{
  if(motor_teg <= 0)
  {
    power_min = 1;
    power_normal = 0;
    power_max = 0;
  }
  if(motor_teg > 0 && motor_teg < 6)
  {
    power_min = (6 - motor_teg) / (6 - 0);
    power_normal = (motor_teg - 0) / (6 - 0);
    power_max = 0;
  }
  if(motor_teg == 6)
  {
    power_min = 0;
    power_normal = 1;
    power_max = 0;
  }
  if(motor_teg > 6 && motor_teg < 12)
  {
    power_min = 0;
    power_normal = (12 - motor_teg) / (12 - 6);
    power_max = (motor_teg - 6) / (12 - 6);
  }
  if(motor_teg >= 12)
  {
    power_min = 0;
    power_normal = 0;
    power_max = 1;
  }
}

// convert base rule into grafik_pwm
void fuzzy_rule()
{  
  float fuzzy_rule_1 = 0;
  float fuzzy_rule_2 = 0;
  float fuzzy_rule_3 = 0;
  float fuzzy_rule_4 = 0;
  float fuzzy_rule_5 = 0;
  float fuzzy_rule_6 = 0;
  float fuzzy_rule_7 = 0;
  float fuzzy_rule_8 = 0;
  float fuzzy_rule_9 = 0;

  float pwm_k_array[3] = {0, 0, 0};
  float pwm_sd_array[4] = {0, 0, 0, 0};
  float pwm_b_array[2] = {0, 0};

  // rule
  fuzzy_rule_1 = min(error_neg, power_min);
  pwm_k_array[0] = fuzzy_rule_1;

  fuzzy_rule_2 = min(error_neg, power_normal);
  pwm_k_array[1] = fuzzy_rule_2;

  fuzzy_rule_3 = min(error_neg, power_max);
  pwm_k_array[3] = fuzzy_rule_3;
  
  fuzzy_rule_4 = min(error_zero, power_min);
  pwm_sd_array[0] = fuzzy_rule_4;
  
  fuzzy_rule_5 = min(error_zero, power_normal);
  pwm_sd_array[1] = fuzzy_rule_5;
  
  fuzzy_rule_6 = min(error_zero, power_max);
  pwm_sd_array[2] = fuzzy_rule_6;
  
  fuzzy_rule_7 = min(error_pos, power_min);
  pwm_b_array[0] = fuzzy_rule_7;
  
  fuzzy_rule_8 = min(error_pos, power_normal);
  pwm_b_array[1] = fuzzy_rule_8;
  
  fuzzy_rule_9 = min(error_pos, power_max);
  pwm_sd_array[3] = fuzzy_rule_9;

  // pilih komponen maksimum pwm
  pwm_k = max(pwm_k_array[0], pwm_k_array[1]);
  pwm_k = max(pwm_k, pwm_k_array[2]);
  
  pwm_sd = max(pwm_sd_array[0], pwm_sd_array[1]);
  pwm_sd = max(pwm_sd, pwm_sd_array[2]);
  pwm_sd = max(pwm_sd, pwm_sd_array[3]);
  
  pwm_b = max(pwm_b_array[0], pwm_b_array[1]);

  float point_k;
  float point_sd_1;
  float point_sd_2;
  float point_b;

  point_k = 127.5 - (pwm_k*127.5);
  point_sd_1 = pwm_sd * (127.5 - 0.0) + 0.0;
  point_sd_2 = 255.0 - (pwm_sd*(255.0 - 12.75));
  point_b = pwm_b * (255.0 - 127.5) + 127.5;

  // grafik pwm_k
  //fuzzy_region
  float pwm_k_graph[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  for(int i=0; i < 9; i++)
  {
    if(fuzzy_region[i] <= point_k)
    {
      pwm_k_graph[i] = pwm_k;
    }
    if(fuzzy_region[i] > point_k)
    {
      pwm_k_graph[i] = 0;
    }
  }

  // grafik pwm_sd
  float pwm_sd_graph[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  for(int i=0; i<9; i++)
  {
    if(fuzzy_region[i] < point_sd_1 || fuzzy_region[i] > point_sd_2)
    {
      pwm_sd_graph[i] = 0;
    }

    if(fuzzy_region[i] >= point_sd_1 && fuzzy_region[i] <= point_sd_2)
    {
      pwm_sd_graph[i] = pwm_sd;
    }
  }

  // grafik pwm_b
  float pwm_b_graph[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  for(int i=0; i<9; i++)
  {
    if(fuzzy_region[i] >= point_b)
    {
      pwm_b_graph[i] = pwm_b;
    }
    if(fuzzy_region[i] < point_b)
    {
      pwm_b_graph[i] = 0;
    }
  }

  // save as grafik_pwm array
  for(int i=0; i<4; i++)
  {
    grafik_pwm[i] = max(pwm_k_graph[i], pwm_sd_graph[i]);
  }
  
  grafik_pwm[4] = max(pwm_k_graph[4], pwm_sd_graph[4]);
  grafik_pwm[4] = max(grafik_pwm[4], pwm_b_graph[4]);

  for(int i=5; i<9; i++)
  {
    grafik_pwm[i] = max(pwm_sd_graph[i], pwm_b_graph[i]);
  }
}

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

