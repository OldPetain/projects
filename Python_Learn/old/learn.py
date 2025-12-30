famous_name = "stalin"
famous_words = "数量产生质量"
print(f'The famous name is  {famous_name}  and his famous words are "{famous_words}" .')

print (3 * 0.1)

age = 19
if age >= 18:
    print("You are old enough to vote.")
else:
    print("You are not old enough to vote.")

user_0 = {
    'username':'efermi',
    'firstname':'enrico',
    'last':'fermi',
}

for key,value in user_0.items():
    print(f"\nKey: {key}")
    print(f"Value: {value}")

def print_models(unprinted_designs,completed_models):
    while unprinted_designs:
        current_design = unprinted_designs.pop()
        print(f"Printing model: {current_design}")
        completed_models.append(current_design)

def show_completed_models(completed_models):
    print("\nThe following models have been printed:")
    for completed_model in completed_models:
        print(completed_model)

unprinted_designs = ['phone case','robot pendant','dodecahedron']
completed_models = []

print_models(unprinted_designs, completed_models)
show_completed_models(completed_models)















