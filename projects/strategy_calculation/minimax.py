from itertools import permutations
from collections import defaultdict

# Генерация всех возможных 4-значных чисел с неповторяющимися цифрами
def generate_all_possible_numbers():
    digits = '0123456789'
    return [''.join(p) for p in permutations(digits, 4)]

# Функция для подсчета быков и коров
def count_bulls_and_cows(secret, guess):
    bulls = 0
    cows = 0
    for i in range(len(secret)):
        for j in range(i, len(guess)):
            if secret[i] == guess[j]:
                if i == j:
                    bulls += 1
                else:
                    cows += 1
    return bulls, cows

# Функция для фильтрации возможных чисел на основе ответа
def filter_possible_numbers(possible_numbers, guess, bulls, cows):
    return [num for num in possible_numbers if count_bulls_and_cows(num, guess) == (bulls, cows)]

# Рекурсивная функция Minimax
def minimax(possible_numbers, depth=0):
    if len(possible_numbers) == 1:
        return 0, possible_numbers[0]  # Если осталось одно число, возвращаем его

    min_max_remaining = float('inf')
    best_guess = None

    # Перебираем все возможные ходы
    for guess in possible_numbers:
        # Группируем возможные числа по ответам (быки и коровы)
        groups = defaultdict(list)
        for num in possible_numbers:
            bulls, cows = count_bulls_and_cows(num, guess)
            groups[(bulls, cows)].append(num)

        # Находим максимальное количество оставшихся чисел для этого хода
        max_remaining = max(len(group) for group in groups.values())

        # Если этот ход лучше, чем предыдущие, обновляем лучший ход
        if max_remaining < min_max_remaining:
            min_max_remaining = max_remaining
            best_guess = guess

    return min_max_remaining, best_guess

# Основная функция для построения дерева решений
def build_decision_tree(possible_numbers):
    if len(possible_numbers) == 1:
        return {'number': possible_numbers[0], 'children': {}}

    _, best_guess = minimax(possible_numbers)
    groups = defaultdict(list)
    for num in possible_numbers:
        bulls, cows = count_bulls_and_cows(num, best_guess)
        groups[(bulls, cows)].append(num)

    children = {}
    for (bulls, cows), group in groups.items():
        children[(bulls, cows)] = build_decision_tree(group)

    return {'guess': best_guess, 'children': children}

# Пример использования
if __name__ == "__main__":
    all_numbers = generate_all_possible_numbers()
    decision_tree = build_decision_tree(all_numbers)

    # Вывод дерева решений (можно адаптировать для визуализации)
    print(decision_tree)