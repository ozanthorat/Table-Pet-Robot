Projenin yüz ifadeleri https://github.com/FluxGarage/RoboEyes adresinden alınmıştır ve ticari bir amaçla kullanılmamıştır.

Projede Direct Port Manipulation kullanılmıştır; yani Arduino'nun standart digitalWrite gibi fonksiyonları yerine doğrudan register tabanlı kodlar yazılmıştır. Kod geliştirmede Antigravity kullanılmıştır.

Robotta ekranla birlikte potansiyometre, ses şiddetini ölçecek sensör, buzzer ve dokunma sensörü bulunmaktadır. Robot Arduino ile yapılmış olup ekran ile arasında I2C protokolü kullanılmıştır.

Potansiyometre: Ekran parlaklık ayarı için kullanılmıştır.

Ses Sensörü: Ses seviyesini dB (desibel) cinsinden ekranda gösterecek şekilde veri sağlamaktadır. Eğer yüksek bir ses algılanırsa ekranda sinirli veya üzgün bir yüz ifadesi belirir ve buzzer tetiklenir. 
Buzzer çalmadan önce ses seviyesi ölçüldüğü için buzzer'ın kendi sesi ölçümü etkilememektedir; ardından dB seviyesi ekranda gösterilir.

Kapasitif Dokunma Sensörü: Dokunulduğunda mutlu bir yüz ifadesi oluşur. Eğer iki kere art arda dokunulursa gözler kalp simgesine dönüşür.

Uyku Modu: Uzun süre etkileşime girilmediğinde robot kendini otomatik olarak uyku moduna alır.
