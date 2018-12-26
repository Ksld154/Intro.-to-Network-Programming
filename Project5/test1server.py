import peewee as pw
import datetime

db_MySQL = pw.MySQLDatabase(
    database = "test1db", 
    host = "test1db.cocusif4zjcb.us-east-1.rds.amazonaws.com", 
    port = 3306, 
    user = "test1admin", 
    passwd = "test1admin"
)

class MySQLModel(pw.Model):
    # """A base model that will use our MySQL database"""
    class Meta:
        database = db_MySQL

class table1(MySQLModel):
    msg = pw.CharField()
    # etc, etc


# when you're ready to start querying, remember to connect
db_MySQL.connect()
theTime = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
res  = table1.create(msg=theTime)

