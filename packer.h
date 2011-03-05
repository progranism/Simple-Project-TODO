struct DATAPACK {
	const char *name;
	const char *data;
	unsigned int len;
};

inline struct DATAPACK find_datapack(struct DATAPACK *packs, const char *name)
{
	while(1)
	{
		if(packs->name == NULL || !strcmp(packs->name, name))
			return *packs;

		packs++;
	}
}

