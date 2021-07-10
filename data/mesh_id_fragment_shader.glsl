#version 430 core

layout(location = 0) in flat uint in_instance_id;

out uint Output;

void main()
{
    Output = in_instance_id;
}