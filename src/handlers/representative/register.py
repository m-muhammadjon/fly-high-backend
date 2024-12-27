from src.database import cursor, DB


async def handle_register(data):
    email = data.get("email")
    if email is None:
        return {"status": "ERROR", "message": "Email is required"}
    else:
        cursor.execute(f"SELECT * FROM representative WHERE email = '{email}'")
        c = cursor.fetchone()
        print(f"{c=}")
        if c:
            return {"status": "ERROR", "message": "User with this email already exists"}

    # Insert the representative into the database
    password = data.get("password")  # Store password as is
    state = data.get("state")
    cursor.execute("SELECT COALESCE(MAX(representative_id), 0) AS last_user_id FROM representative")
    result = cursor.fetchone()
    last_user_id = result[0]
    new_user_id = last_user_id + 1
    cursor.execute(
        "INSERT INTO representative (representative_id, email, password, state) VALUES (%s, %s, %s, %s)",
        (new_user_id, email, password, state)
    )
    DB.commit()
    representative_id = new_user_id

    cursor.execute("SELECT COALESCE(MAX(airline_id), 0) AS last_user_id FROM airline")
    result2 = cursor.fetchone()
    last_user_id2 = result2[0]
    new_user_id2 = last_user_id2 + 1
    cursor.execute(
        """
        INSERT INTO airline (airline_id, representative_id, name, logo, contact_number, contact_email, address, tin, 
                             account_holder_name, bank_name, bank_swift_code, bank_account_number, airline_description)
        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """,
        (
            new_user_id2,
            representative_id,
            data.get("name", "name"),
            data.get("logo", "logo"),
            data.get("contact_number", ""),
            data.get("contact_email", ""),
            data.get("address", ""),
            data.get("tin", ""),
            data.get("account_holder_name", ""),
            data.get("bank_name", ""),
            data.get("bank_swift_code", ""),
            data.get("bank_account_number", ""),
            data.get("airline_description", "")
        )
    )
    DB.commit()
    return {"status": "WAIT", "result": {"token": None}}
