import boto3
import time

client = boto3.client('ec2')

user_data = '''#!/bin/bash
cd /home/ubuntu
echo 'test' > /home/ubuntu/commandline-output.txt
python3.6 ./hello_world.py > /home/ubuntu/python.txt
python3.6 ./server_app.py'''


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
    created_instance_id = instance['InstanceId']
print(created_instance_id)


time.sleep(10)

# Connect to EC2
ec2 = boto3.client('ec2')

target_ip = ec2.describe_instances(InstanceIds=[created_instance_id])['Reservations'][0]['Instances'][0]['PublicIpAddress']
print(target_ip)

