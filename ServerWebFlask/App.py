# Import the Flask class and render_template function from the Flask library
from flask import Flask, render_template

# Import the MariaDB connector library to interact with the MariaDB/MySQL database
import mariadb

# Create a Flask application instance
app = Flask(__name__)

# -------------------- DATABASE CONFIGURATION --------------------
DB_USER = "user"        # Database username
DB_PASS = "2001"        # Database password
DB_HOST = "localhost"   # Database host (localhost means the same machine)
DB_NAME = "IMAGE"       # Name of the database to connect to

# -------------------- FUNCTION TO FETCH MESSAGES --------------------
def get_messages():
    """
    Connects to the MariaDB database, retrieves all rows from the 'messages' table,
    ordered by 'id' descending (latest first), and returns them as a list.
    """
    # Establish a connection to the MariaDB database using the configuration above
    conn = mariadb.connect(
        user=DB_USER,
        password=DB_PASS,
        host=DB_HOST,
        database=DB_NAME
    )

    # Create a cursor object to execute SQL queries
    cur = conn.cursor()

    # Execute a SQL query to select all columns from 'messages' table, ordered by id descending
    cur.execute("SELECT id, topic, payload, timestamp FROM messages ORDER BY id DESC")

    # Fetch all rows returned by the query
    rows = cur.fetchall()

    # Close the cursor to free resources
    cur.close()

    # Close the database connection
    conn.close()

    # Return the list of rows to the caller
    return rows

# -------------------- FLASK ROUTE --------------------
@app.route("/")  # Define the route for the root URL "/"
def index():
    """
    Route handler for the homepage. Fetches all messages from the database
    and renders them using the 'index.html' template.
    """
    # Call the function to get all messages from the database
    messages = get_messages()

    # Render the 'index.html' template, passing the messages to it
    return render_template("index.html", messages=messages)

# -------------------- MAIN ENTRY POINT --------------------
if __name__ == "__main__":
    # Start the Flask development server on all network interfaces (0.0.0.0)
    # and port 5000, with debug mode enabled
    app.run(host="0.0.0.0", port=5000, debug=True)
