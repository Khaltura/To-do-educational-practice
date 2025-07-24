from sqlalchemy.orm import Session
import models, schemas
import hashlib 

def hash_password(password: str) -> str: 
    return hashlib.sha256(password.encode()).hexdigest()

def create_user(db: Session, user: schemas.UserCreate): 
    hashed = hash_password(user.password)
    db_user = models.User(username = user.username, password_hash = hashed)
    db.add(db_user)
    db.commit()
    db.refresh(db_user)
    return db_user

def authenticate_user(db: Session, username:str, password: str): 
    user = db.query(models.User).filter(models.User.username == username).first()
    if user and user.password_hash == hash_password(password):
        return user
    return None

def create_task(db: Session, user_id: int, task: schemas.TaskCreate):
    db_task = models.Task(**task.dict(), user_id = user_id)
    db.add(db_task)
    db.commit()
    db.refresh(db_task)
    return db_task

def get_tasks(db: Session, user_id: int): 
    return db.query(models.Task).filter(models.Task.user_id == user_id).all()