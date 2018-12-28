import boto3

client = boto3.client(
    'ec2' 
)

user_data = '''#!/bin/bash
cd /home/ubuntu
echo 'test' > /home/ubuntu/commandline-output.txt
python3.6 ./hello_world.py > /home/ubuntu/python.txt
python3.6 ./server_app.py'''

# user_data = """#cloud-boothook 
# #!/bin/bash 
# cd /home/ubuntu
# echo 'test' > /home/ubuntu/user-data-commandline-output.txt"""


resp = client.run_instances(
    ImageId='ami-0b95686768d9b1890',      # my AMI(with trying sys.path.append to import)
    # ImageId='ami-041383eec70d1c465',    # my AMI(with app_server.py)
    # ImageId='ami-09ba593e2c03395d5',  # my AMI(for testing)
    # ImageId='ami-0ac019f4fcb7cb7e6',  # ubuntu
    InstanceType='t2.micro',
    MinCount=1,
    MaxCount=1,
    SecurityGroupIds=['sg-0100b5a9d0143c2fd'],
    KeyName='test1',
    UserData=user_data
)

for instance in resp['Instances']:
    print(instance['InstanceId'])