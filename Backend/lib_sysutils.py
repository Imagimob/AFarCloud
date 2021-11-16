# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Various small useful functions




def get_env_variables_if_exists(env_var_name, type=str,default=None):
    '''

    Get an environmental variable by name if it exists or return a default value if it doesn't

    :param env_var_name: Name of the env viriable to read
    :param type: Type of the env variable to read
    :param default: A default value to return if the name is not defined in the environment.
    :return: the value for the variable
    '''

    import os


    if not env_var_name in os.environ.keys():
        if default is None:
            raise EnvironmentError("Variable " + env_var_name + " not defined")
        return type(default)
    return type(os.environ[env_var_name])


def dict_compare(d1, d2, eps=None):
    d1_keys = set(d1.keys())
    d2_keys = set(d2.keys())
    if not d1_keys==d2_keys:
        return False
    
    if eps is None: 
        if len([k for k in d1_keys if d1[k]!=d2[k]])>0:
            return False
    else:
        if len([k for k in d1_keys if abs(d1[k]-d2[k])>eps ])>0:
            return False
    return True

