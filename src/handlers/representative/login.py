from src.database import cursor
from src.utils import get_token


async def handle_login(details):
    """
    Handles the login request from the client.
    """
    email = details.get("email")
    password = details.get("password")
    column_names = ["representative_id", "email"]
    cursor.execute(
        f"SELECT {','.join(column_names)} FROM representative WHERE email = '{email}' AND password = '{password}'",
    )
    user = cursor.fetchone()

    if user:
        user_dict = dict(zip(column_names, user))
        token = await get_token(user_dict.get("representative_id"))
        return {"status": "OK", "result": {"token": token}}
    return {"status": "ERROR", "result": {"message": "Invalid credentials"}}
