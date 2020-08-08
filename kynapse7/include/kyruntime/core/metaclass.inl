/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY

template <typename TheClass, typename Ctor>
MetaClass<TheClass, Ctor>::~MetaClass()
{
}

template <typename TheClass, typename Ctor>
KyBool MetaClass<TheClass, Ctor>::Register(MetaClass& cls)
{
	KY_FUNCTION("Kaim::MetaClass<TheClass,Ctor>::Register");
	
	Holder& holder = GetHolder();
	const char* name = cls.GetName();
	for (KyInt32 i=0;i<holder.m_classeCount;++i)
	{
		if (strcmp(holder.m_classes[i]->m_name,name)==0)
		{
			holder.m_classes[i] = &cls;
			return KY_TRUE;
		}
	}
	if (holder.m_classeCount == KYNO_MAX_METACLASS_COUNT)
	{
		KY_MESSAGE(KY_LOG_ERROR, ("Max number (%d) of meta classes overflow (%s)", KYNO_MAX_METACLASS_COUNT, cls.GetName()));
		return KY_FALSE;
	}
	holder.m_classes[holder.m_classeCount] = &cls;
	++holder.m_classeCount;
	return KY_TRUE;
}

template <typename TheClass, typename Ctor>
void MetaClass<TheClass, Ctor>::UnRegisterAll()
{
	GetHolder().Clear();
}

template <typename TheClass, typename Ctor>
MetaClass<TheClass, Ctor>::MetaClass(const char* className, Ctor ctr, MetaClass* baseClass)
{
	m_name = className;
	if (Register(*this) == KY_FALSE)
		m_id = -1;
	else
		m_id = GetHolder().m_classeCount-1;
	m_constructor = ctr;
	m_baseClass = baseClass;
}

template <typename TheClass, typename Ctor>
KyBool MetaClass<TheClass, Ctor>::IsA(const MetaClass& cls) const
{
	const MetaClass<TheClass, Ctor>* pCls = this;
	do
	{
		if (&cls == pCls)
			return KY_TRUE;
		pCls = pCls->m_baseClass;
	}
	while (pCls != NULL);
	return KY_FALSE;
}

template <typename TheClass, typename Ctor>
MetaClass<TheClass, Ctor>* MetaClass<TheClass, Ctor>::GetClassByName(const char* name)
{
	if (name==NULL)
		return (MetaClass<TheClass,Ctor>*)NULL;
	Holder& holder = GetHolder();
	for (KyInt32 i=0;i<holder.m_classeCount;++i)
		if (strcmp(holder.m_classes[i]->m_name,name)==0)
			return holder.m_classes[i];
	return (MetaClass*) NULL;
}

