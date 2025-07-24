from fastapi import FastAPI, Depends, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session
import models, schemas, crud
from database import SessionLocal, engine
import database

app = FastAPI()
database.Base.metadata.create_all(bind = database.engine)

#нужно будет переуказать домены
app.add_middleware(
    CORSMiddleware,
    allow_origins = ["*"],
    allow_credentials = True, 
    allow_methods = ["*"], 
    allow_headers = ["*"],
)

@app.get("/")
def read_root():
    return {"message": "Hello, world!"}

def get_db(): 
    db = database.SessionLocal()
    try: 
        yield db 
    finally: 
        db.close()

@app.post("/register")
def register(user: schemas.UserCreate, db: Session = Depends(get_db)):
    return crud.create_user(db, user)

@app.post("/login")
def login(user: schemas.UserLogin, db: Session = Depends(get_db)):
    db_user = crud.authenticate_user(db, user.username, user.password)
    if not db_user:
        raise HTTPException(status_code = 401, detail = "Invalid credentials")
    return {"id": db_user.id, "username": db_user.username}

@app.post("/tasks/", response_model = schemas.TaskOut)
def create_task(task: schemas.TaskCreate, user_id: int, db: Session = Depends(get_db)):
    return crud.create_tasks(db, user_id, task)

@app.get("/tasks", response_model = list[schemas.TaskOut])
def get_tasks(db: Session = Depends(get_db)):
    return db.query(models.Task).all()

