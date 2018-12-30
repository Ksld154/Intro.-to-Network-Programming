import boto3
import time






target_instance_id = 'i-0a5e8135a0a2b4fd0'
# ec2 = boto3.resource('ec2')
# ec2.instances.filter(InstanceIds=target_instance_id).terminate()


client = boto3.client('ec2')
response = client.terminate_instances(InstanceIds=[target_instance_id])
print(response)