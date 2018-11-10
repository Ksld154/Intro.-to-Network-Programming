import peewee as pw


#db = pw.MySQLDatabase(database="lylin1997_cs_NP_Project3", host="dbhome.cs.nctu.edu.tw", user='lylin1997_cs', passwd='lylin1997db')
sqlite_db = pw.SqliteDatabase('test1.db')

class SQLiteModel(pw.Model):
    """A base model that will use our SQLite database"""
    class Meta:
        database = sqlite_db

class t2(SQLiteModel):
    username = pw.CharField(unique=True)
    password = pw.CharField()
    
    @classmethod
    def Insert_User(self, account, password):
        try:
            t2.create(username=account, password=password)    
        except pw.IntegrityError as e:
            if e.args[0] == "UNIQUE constraint failed: t2.username":
                print("%s is already used" %account)
                #raise Exception('username already exists')
                #print(e.args[0])
                #print("UNIQUE constraint failed: t2.username")


def main():
    t2.create_table()
    t2.Insert_User('Mark2','test1')


if __name__ == '__main__':
    main()









#r2 = t2.create(username='Charlie', password='c1')
#print(r1)
#print(r2)
#t1.create(username='Charlie', password='c1')

#db.connect()