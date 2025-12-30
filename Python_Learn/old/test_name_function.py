from Python_Learn.name_function import get_formatted_name

def test_first_last_name():
    """能够正确地处理像Janis Joplin这样的姓名吗？"""
    formatted_name = get_formatted_name("Janis", "Joplin")
    assert formatted_name == "Janis Joplin"

def test_first_middle_last_name():
    """能够正确地处理像John Smith这样的姓名吗？"""
    formatted_name = get_formatted_name("John", "Smith", "Johnson")
    assert formatted_name == "John Johnson Smith"