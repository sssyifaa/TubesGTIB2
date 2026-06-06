# Gilang's Love Simulator

## Deskripsi

Gilang's Love Simulator adalah game simulasi 3D berbasis OpenGL yang dikembangkan sebagai proyek mata kuliah Grafik dan Teknik Interaktif.

Pemain berperan sebagai Gilang, seorang pemuda yang ingin mendapatkan hati seorang gadis desa bernama Tisya. Untuk meningkatkan hubungan mereka, pemain harus menyelesaikan berbagai misi yang diberikan. Setiap misi yang berhasil diselesaikan akan meningkatkan status hubungan, sedangkan kegagalan menyelesaikan misi dalam batas waktu yang ditentukan akan menurunkan hubungan mereka.

Game ini mengimplementasikan berbagai konsep grafika komputer seperti transformasi, proyeksi, animasi, pencahayaan, tekstur, kamera, dan collision detection menggunakan OpenGL dan GLUT.

---

## Alur Permainan

Pemain akan menjalani beberapa tahapan hubungan dengan Tisya:

- Stranger
- Pacaran
- Pertunangan
- Menikah

Untuk mencapai tahap akhir, pemain harus menyelesaikan tiga misi utama secara berurutan:

1. Mengumpulkan apel.
2. Mengumpulkan telur ayam.
3. Memerah susu sapi.

Setiap misi memiliki batas waktu tertentu yang harus dipenuhi pemain. 

---

## Fitur Utama

### Karakter 3D
- Karakter utama Gilang.
- Karakter Tisya sebagai tujuan utama permainan.
- NPC Si Bro sebagai pemberi informasi.
- Hewan ternak berupa ayam dan sapi.

### Sistem Misi
- Misi pengumpulan apel.
- Misi pengumpulan telur.
- Misi pemerahan susu sapi.
- Sistem progres hubungan berdasarkan penyelesaian misi.

### Sistem Interaksi
- Interaksi dengan NPC.
- Pengambilan item.
- Interaksi dengan kandang dan gerbang.
- Sistem timer dan objektif permainan.

### Lingkungan 3D
- Desa dengan rumah-rumah procedural.
- Jalan utama.
- Area perkebunan apel.
- Kandang ayam.
- Kandang sapi.
- Area eksplorasi pemain.

### Implementasi Grafika Komputer
- Primitive Drawing
- Transformasi Translasi
- Transformasi Rotasi
- Proyeksi Perspektif
- Animasi Karakter
- Third-Person Camera
- Depth Testing
- Lighting
- Texture Mapping
- Shadow Rendering
- Collision Detection

---

## Kontrol

| Tombol  | Fungsi                 |
|---------|------------------------|
| W       | Bergerak maju          |
| S       | Bergerak mundur        |
| A       | Bergerak ke kiri       |
| D       | Bergerak ke kanan      |
| Mouse   | Mengatur kamera        |
| Space   | Melompat               |
| E       | Interaksi dengan objek |

---

## Teknologi yang Digunakan

- C++
- OpenGL
- GLUT / FreeGLUT

---

## Cara Menjalankan Program

### Persyaratan

Pastikan telah terpasang:

- Compiler C++
- OpenGL
- GLUT / FreeGLUT

### Langkah Menjalankan

1. Clone atau download project.
2. Buka project menggunakan IDE yang mendukung C++.
3. Pastikan seluruh library telah terhubung.
4. Compile project.
5. Jalankan program.

---

## Struktur Proyek

```
Gilangs-Love-Simulator/
│
├── mainApanih.cpp
│
├── player.cpp
├── player.h
│
├── tisya.cpp
├── tisya.h
│
├── bro.cpp
├── bro.h 
│
├── camera.cpp
├── camera.h 
│
├── input.cpp
├── input.h
│
├── mission.cpp
├── mission.h 
│
├── collision.cpp
├── collision.h
│
├── house.cpp
├── house.h 
│
├── chiken.cpp
├── chiken.h
│
├── chikenshed.cpp
├── chikenshed.h
│
├── cow.cpp
├── cow.h  
│
├── cowshed.cpp
├── cowshed.h
│
├── environment.cpp
├── environment.h
│
├── texture.cpp
├── texture.h
├── shadow.cpp
├── shadow.h
│
├── global.cpp
├── global.h
│
├── utils.cpp
├── utils.h
│
├── Makefile.win
│
├── ProjectApanih.dev
├── ProjectApanih.layout
│
└── README.md
```

---

## Tujuan Pembelajaran

Proyek ini dikembangkan untuk mengimplementasikan konsep-konsep yang dipelajari pada mata kuliah Grafika dan Teknik Interaktif, khususnya:

- Pemodelan objek 3D.
- Rendering menggunakan OpenGL.
- Animasi karakter.
- Sistem kamera.
- Interaksi pengguna.
- Pengelolaan lingkungan virtual.
- Teknik pencahayaan dan tekstur.

---

## Pengembang Game

Tugas Besar Mata Kuliah Grafik dan Teknik Interaktif

Program Studi Informatika  
Universitas Diponegoro

Kelompok 05 - Kelas B

1. Galang Bintang Ramadhan - 24060124130101
2. Puti Shasta Khafiyani - 24060124140132
3. Revalina Salwa Aliya Wicaksono Prabowo - 24060124140155
4. Syifa Aeni Mudrikah - 24060124120043

