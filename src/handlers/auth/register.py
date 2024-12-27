from src.database import cursor, DB
from src.utils import get_token


async def handle_register_user(details):
    """
    Handles the registration request from the client.
    """

    cursor.execute("SELECT COALESCE(MAX(user_id), 0) AS last_user_id FROM users")
    result = cursor.fetchone()
    last_user_id = result[0]
    new_user_id = last_user_id + 1
    insert_query = """
                INSERT INTO users (
                    user_id, email, password, last_name, first_name, phone_number, 
                    birthdate, passport, citizenship, address_country, 
                    address_city, address_state, address_line_1, user_state
                )
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, 'ACTIVE')
            """
    values = (
        new_user_id,
        details.get("email"),
        details.get("password"),
        details.get("last_name"),
        details.get("first_name"),
        details.get("phone_number"),
        details.get("birthdate"),
        details.get("passport"),
        details.get("citizenship"),
        details.get("address_country"),
        details.get("address_city"),
        details.get("address_state"),
        details.get("address_line_1")
    )
    cursor.execute(insert_query, values)
    DB.commit()

    token = await get_token(new_user_id)
    return {"status": "OK", "result": {"token": token}}
