
from itertools import product


registry = {}


class ConfigInitError(Exception):
    def __init__(self, arg):
        Exception.__init__(self,
            'toolchains.Config: Argument `%s` is mandatory.' % arg)

class ConfigDupError(Exception):
    def __init__(self, key):
        Exception.__init__(self,
            'toolchains.Config: \'%s\' duplicated key.' % str(key))


class ArgNotSupportedError(Exception):
    def __init__(self, toolchain=None, target_os=None, compiler=None):
        self.toolchain = toolchain
        self.target_os = target_os
        self.compiler = compiler

        self.err = (
            ('toolchain=\'%s\'' % toolchain) if toolchain else
            ('target-os=\'%s\' compiler=\'%s\'' % (target_os, compiler))
        ) + ' not supported.'
        self.help = ('Supported toolchains are:\n  ' +
            '\n  '.join(sorted(t.id for t in set(registry.values()))))


class Config:

    def __init__(self, id, target_os, compiler,
                 alt_id = [], alt_target_os = [], alt_compiler = [],
                 prefix = '',
                 **env):

        if not id:
            raise ConfigInitError('id')
        if not target_os:
            raise ConfigInitError('target_os')
        if not compiler:
            raise ConfigInitError('compiler')

        self._id = (id,)+tuple(alt_id)
        self._target_os = (target_os,)+tuple(alt_target_os)
        self._compiler = (compiler,)+tuple(alt_compiler)
        self.prefix = prefix
        self._env = env

        # Check dup.
        for k in self.keys:
            if registry.get(k, None):
                raise ConfigDupError(k)
            else:
                registry[k] = self


    def __str__(self):
        return self._id[0]

    def log(self):
        return (
            'toolchain='+str(self._id),
            'target-os='+str(self._target_os),
            'compiler='+str(self._compiler),
            'prefix='+str(self.prefix),
            'env='+str(self._env),
        )

    @property
    def keys(self):
        return self._id+tuple(product(self._target_os, self._compiler))

    @property
    def id(self):
        return self._id[0]

    @property
    def target_os(self):
        return self._target_os

    @property
    def compiler(self):
        return self._compiler

    def env(self, name):
        return self._env.get(name, '')

    @staticmethod
    def find_with_args(**args):
        id = args.get('toolchain', '')
        if id:
            toolchain = registry.get(id, None)
            if toolchain:
                return toolchain
            else:
                raise ArgNotSupportedError(toolchain=id)

        else:
            target_os = args.get('target-os', '')
            compiler = args.get('compiler', '')
            toolchain = registry.get((target_os,compiler), None)
            if toolchain:
                return toolchain
            else:
                raise ArgNotSupportedError(target_os=target_os, compiler=compiler)
