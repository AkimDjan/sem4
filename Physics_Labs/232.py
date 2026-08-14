import numpy as np
import matplotlib.pyplot as plt
import os

# --- ДАННЫЕ (ваши) ---
no_phase_degrees = np.arange(0, 190, 10)
no_phase = np.array([0, 0.07, 0.18, 0.37, 0.62, 0.85, 1.06, 1.22, 1.32, 1.34, 1.28, 1.14, 0.95, 0.75, 0.52, 0.27, 0.12, 0.02, 0.01])

degreeses_step_10 = np.array([40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220])
dark_mirror = np.array([0.01, 0.01, 0.02, 0.04, 0.08, 0.14, 0.19, 0.24, 0.28, 0.30, 0.31, 0.30, 0.27, 0.23, 0.18, 0.13, 0.07, 0.04, 0.01])

brust_50_degreeses = np.array([15, 20, 25, 30, 35, 40, 45, 50, 55, 56, 57, 58, 59, 60, 65, 70, 75, 80])
bruster_min = np.array([0.11, 0.11, 0.1, 0.09, 0.08, 0.07, 0.06, 0.05, 0.04, 0.04, 0.04, 0.04, 0.05, 0.05, 0.06, 0.13, 0.43, 0.48])

bruster_140_degrees = np.array([15, 20, 25, 30, 35, 40, 45, 50, 55, 56, 57, 58, 59, 60, 65, 70, 75, 80])
bruster_max = np.array([0.12, 0.13, 0.14, 0.15, 0.16, 0.18, 0.21, 0.23, 0.29, 0.3, 0.3, 0.32, 0.35, 0.34, 0.45, 0.62, 0.86, 0.86])

plast_degrees = np.arange(50, 240, 10)
plast = np.array([0., 0.01, 0.03, 0.06, 0.09, 0.12, 0.15, 0.17, 0.19, 0.19, 0.19, 0.17, 0.14, 0.11, 0.08, 0.05, 0.03, 0.01, 0.01])

# ---------------------------
# НАСТРОЙКИ СТИЛЯ И ПАРАМЕТРОВ
# ---------------------------
SAVE_DIR = "imgs"
os.makedirs(SAVE_DIR, exist_ok=True)

plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams.update({
    'font.size': 12,
    'axes.titlesize': 14,
    'axes.labelsize': 12,
    'lines.linewidth': 2,
    'lines.markersize': 6,
    'legend.frameon': True,
    'legend.edgecolor': 'black'
})

COLOR_EXP = '#E24A33'
COLOR_THEORY = '#348ABD'
COLOR_EXP2 = '#988ED5'
COLOR_THEORY2 = '#8EBA42'

# НОВАЯ разумная функция погрешностей (5% + небольшой фон)
def experimental_error(I):
    return 0.05 * np.abs(I) + 0.01

def malus_theory(I0, angles, phi0=0):
    return I0 * (np.cos(np.radians(angles - phi0)))**2

def plot_with_errors(x, y, y_err, label, color):
    plt.errorbar(x, y, yerr=y_err, fmt='o', capsize=3, capthick=1.5,
                 label=label, color=color, markeredgecolor='black', zorder=5)

def plot_theory_curve(x, y, label, color):
    plt.plot(x, y, marker='s', markersize=3, linestyle='none',
             color=color, label=label, alpha=0.7)

def fresnel_reflection(theta_deg, n=1.5):
    theta = np.radians(theta_deg)
    sin_t = np.sin(theta) / n
    sin_t = np.clip(sin_t, -1, 1)
    theta_t = np.arcsin(sin_t)
    R_perp = (np.sin(theta - theta_t) / np.sin(theta + theta_t))**2
    R_parallel = (np.tan(theta - theta_t) / np.tan(theta + theta_t))**2
    return R_perp, R_parallel

def save_plot(name):
    plt.tight_layout()
    plt.savefig(f"{SAVE_DIR}/{name}.png", dpi=300)
    plt.close()

# ==========================================
# 1. Задание 1: Закон Малюса (прямой луч)
# ==========================================
plt.figure(figsize=(8, 5))
plt.title("Задание 1: Проверка закона Малюса")

angles = no_phase_degrees
I0 = np.max(no_phase)
phi0 = 90   # максимум при 90° по данным, это правильный сдвиг

angles_dense = np.arange(0, 185, 5)
theory = malus_theory(I0, angles_dense, phi0)

plot_with_errors(angles, no_phase, experimental_error(no_phase), "Эксперимент", COLOR_EXP)
plot_theory_curve(angles_dense, theory, r"Теория $I = I_0 \cos^2(\alpha)$", COLOR_THEORY)

plt.xlabel(r"Угол поворота анализатора, $\alpha$ (градусы)")
plt.ylabel("Освещённость, $I$ (Лк)")
plt.legend()
plt.xlim(0, 180)
plt.ylim(0)
save_plot("task1_malus_law")

# ==========================================
# 2. Задание 2а: Черное зеркало (зависимость от поляризации)
# ==========================================
plt.figure(figsize=(8, 5))
plt.title("Задание 2а: Зависимость от плоскости поляризации")

# Определяем угол, при котором отражение максимально
idx_max = np.argmax(dark_mirror)
phi_max = degreeses_step_10[idx_max]   # у вас ~140°

plot_with_errors(degreeses_step_10, dark_mirror, experimental_error(dark_mirror), "Отраженный свет", COLOR_EXP)

# Строим теоретическую огибающую с правильным сдвигом
angles_dense_2a = np.arange(30, 230, 5)   # покрываем диапазон ваших данных
A = np.min(dark_mirror)
B = np.max(dark_mirror) - A
theory_mirror = A + B * (np.cos(np.radians(angles_dense_2a - 140)))**2
plot_theory_curve(angles_dense_2a, theory_mirror, "Теоретическая огибающая", COLOR_THEORY)

plt.xlabel("Угол поворота поляризатора (градусы)")
plt.ylabel("Освещённость (Лк)")
plt.legend()
save_plot("task2a_dark_mirror")

# ==========================================
# 3. Задание 2б: Формулы Френеля и угол Брюстера
# ==========================================
plt.figure(figsize=(8, 5))
plt.title("Задание 2б: Отражение по Френелю и угол Брюстера")

# Определяем угол Брюстера по минимуму параллельной компоненты
min_idx = np.argmin(bruster_min)
brewster_angle = brust_50_degreeses[min_idx]
n_calc = np.tan(np.radians(brewster_angle))

angles_dense = np.arange(10, 80, 1.5)
R_perp, R_parallel = fresnel_reflection(angles_dense, n=n_calc)

# Масштабирующие коэффициенты: подгоняем по начальным точкам
# Перпендикулярная компонента
scale_perp = np.mean(bruster_max[:3] / R_perp[:3])   # среднее по первым трём углам
# Параллельная компонента
scale_par = np.mean(bruster_min[:3] / R_parallel[:3])

plot_with_errors(bruster_140_degrees, bruster_max, experimental_error(bruster_max),
                 "Перпендикулярная пол. (Эксп)", COLOR_EXP)
plot_theory_curve(angles_dense, R_perp * scale_perp,
                  "Перпендикулярная (Теория)", COLOR_THEORY)

plot_with_errors(brust_50_degreeses, bruster_min, experimental_error(bruster_min),
                 "Параллельная пол. (Эксп)", COLOR_EXP2)
plot_theory_curve(angles_dense, R_parallel * scale_par,
                  "Параллельная (Теория)", COLOR_THEORY2)

plt.axvline(brewster_angle, color='gray', linestyle='--', linewidth=1.5,
            label=fr"Угол Брюстера $\theta_B = {brewster_angle:.1f}^\circ$" + "\n" +
                  fr"Показ. прел. $n = \tan\theta_B \approx {n_calc:.2f}$")
plt.xlabel(r"Угол падения, $\theta$ (градусы)")
plt.ylabel("Освещённость (Лк)")
plt.legend()
save_plot("task2b_fresnel_brewster")

# ==========================================
# 4. Задание 2в: Закон Малюса для отраженного луча
# ==========================================
plt.figure(figsize=(8, 5))
plt.title("Задание 2в: Поляризация отраженного света")

# Снова находим угол максимума в данных (у вас ~130-150°)
idx_max_plast = np.argmax(plast)
phi_plast = plast_degrees[idx_max_plast]   # примерно 130°

plot_with_errors(plast_degrees, plast, experimental_error(plast),
                 "Отраженный луч (Эксп)", COLOR_EXP)

# Теоретическая кривая в том же диапазоне углов, что и эксперимент
angles_dense2 = np.arange(40, 250, 5)
A_plast = np.min(plast)
B_plast = np.max(plast) - A_plast
theory_plast = A_plast + B_plast * (np.cos(np.radians(angles_dense2 - 140)))**2

plot_theory_curve(angles_dense2, theory_plast,
                  "Закон Малюса (Теория)", COLOR_THEORY)

plt.xlabel("Угол поворота анализатора (градусы)")
plt.ylabel("Освещённость (Лк)")
plt.xlim(40, 240)   # обрезаем по экспериментальным точкам
plt.legend()
save_plot("task2c_reflected_malus")

print("Готово! Погрешности уменьшены, сдвиги подобраны под данные.")