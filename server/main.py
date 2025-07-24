from fastapi import FastAPI, Depends, HTTPException, status
from fastapi.security import OAuth2PasswordRequestForm
from sqlalchemy.orm import Session
from database import get_db, engine
import models, schemas, crud
from auth import get_password_hash, verify_password, create_access_token, get_current_user, get_user_by_username
from datetime import timedelta

models.Base.metadata.create_all(bind=engine)

app = FastAPI()

@app.post("/register")
def register(user: schemas.UserCreate, db: Session = Depends(get_db)):
    if get_user_by_username(db, user.username):
        raise HTTPException(status_code=400, detail="User already exists")
    hashed_pw = get_password_hash(user.password)
    return crud.create_user(db, user.username, hashed_pw)

@app.post("/token", response_model=schemas.Token)
def login(form_data: OAuth2PasswordRequestForm = Depends(), db: Session = Depends(get_db)):
    db_user = get_user_by_username(db, form_data.username)
    if not db_user or not verify_password(form_data.password, db_user.password_hash):
        raise HTTPException(status_code=401, detail="Invalid credentials")
    access_token = create_access_token(
        data={"sub": db_user.username},
        expires_delta=timedelta(minutes=30)
    )
    return {"access_token": access_token, "token_type": "bearer"}

@app.post("/tasks", response_model=schemas.TaskOut)
def create_task(task: schemas.TaskCreate, db: Session = Depends(get_db), user=Depends(get_current_user)):
    return crud.create_task(db, task, user.id)

@app.get("/tasks", response_model=list[schemas.TaskOut])
def read_tasks(db: Session = Depends(get_db), user=Depends(get_current_user)):
    return crud.get_tasks(db, user.id)

@app.delete("/tasks/{task_id}")
def delete_task(task_id: int, db: Session = Depends(get_db), user=Depends(get_current_user)):
    if not crud.delete_task(db, task_id, user.id):
        raise HTTPException(status_code=404, detail="Task not found")
    return {"detail": "Deleted"}

@app.patch("/tasks/{task_id}", response_model=schemas.TaskOut)
def update_task(task_id: int, updates: dict, db: Session = Depends(get_db), user=Depends(get_current_user)):
    task = crud.update_task(db, task_id, user.id, updates)
    if not task:
        raise HTTPException(status_code=404, detail="Task not found")
    return task

@app.post("/notes", response_model=schemas.NoteOut)
def create_note(note: schemas.NoteCreate, db: Session = Depends(get_db), user=Depends(get_current_user)):
    return crud.create_note(db, note, user.id)

@app.get("/notes", response_model=list[schemas.NoteOut])
def read_notes(db: Session = Depends(get_db), user=Depends(get_current_user)):
    return crud.get_notes(db, user.id)

@app.delete("/notes/{note_id}")
def delete_note(note_id: int, db: Session = Depends(get_db), user=Depends(get_current_user)):
    if not crud.delete_note(db, note_id, user.id):
        raise HTTPException(status_code=404, detail="Note not found")
    return {"detail": "Deleted"}

@app.patch("/notes/{note_id}", response_model=schemas.NoteOut)
def update_note(note_id: int, content: str, db: Session = Depends(get_db), user=Depends(get_current_user)):
    note = crud.update_note(db, note_id, user.id, content)
    if not note:
        raise HTTPException(status_code=404, detail="Note not found")
    return note