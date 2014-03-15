#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <string>
using namespace std;
//一个特殊的指数，用于标识链表头。
#define EXP_HEAD_NODE -1234
//链表节点结构体
struct node{
	double factor;
	int exp;
	node * next;
};
//单节点深拷贝
node * nodecpy(node * src){
	node *new_node=new node;
	new_node->factor=src->factor;
	new_node->exp=src->exp;
	new_node->next=src->next;
	return new_node;
}
//建立新的单节点
node * new_node(double factor,int exp){
	node * n=new node;
	n->factor=factor;
	n->exp=exp;
	n->next=NULL;
	return n;
}
//整条链表的深拷贝
node * nodescpy(node * src){
	if(!src)return NULL;
	node * new_head=nodecpy(src);
	node * new_node=new_head;
	while(src=src->next){
		new_node->next=nodecpy(src);
		new_node=new_node->next;
	}
	return new_head;
}
//整条链表的数乘
void nodesScalarMultiply(node * src,double factor_multiply,int exp_add){
	do{
		if((!src) || (src->exp==EXP_HEAD_NODE))continue;
		src->factor*=factor_multiply;
		src->exp+=exp_add;
	}while(src=src->next);
}
//删除下一个节点
void remove_next(node * A){
	node * tmp=A->next;
	if(tmp){
		A->next=tmp->next;
		delete(tmp);
	}
}
//整条链表求微分
void nodesDiff(node * src){
	node * src_prev=src;
	while(src=src->next){
		src->factor*=src->exp;
		src->exp=src->exp-1;
		if(src->factor==0){
			remove_next(src_prev);
			src=src_prev;
		}
		src_prev=src;
	};
}
//整条链表拷贝并乘上x的n次方
node * nodescpy_exp_add(node * src,int exp_added){
	node * new_src=nodescpy(src);
	nodesScalarMultiply(new_src,1,exp_added);
	return new_src;
}
//整条链表拷贝并乘上-1
node * nodescpyNegation(node * src){
	node * new_src=nodescpy(src);
	nodesScalarMultiply(new_src,-1,0);
	return new_src;
}
//整条链表释放内存
void nodesfree(node * head){
	while(head){
		node * tmp=head;
		head=head->next;
		delete(head);
	}
}
//插入节点
void insert(node * A,node * B){

	B->next=A->next;
	A->next=B;
}
//插入排序，并合并同次数节点。
void insert_and_sort(node * head,node * new_node,bool add_once=false){
	if(new_node->exp==EXP_HEAD_NODE)new_node=new_node->next;
	node * i=head->next;
	node * i_prev=head;
	while(new_node){
		while(i &&(i->exp)>(new_node->exp)){
			i_prev=i;
			i=i->next;
		}
		if(i && (i->exp==new_node->exp)){
			i->factor+=new_node->factor;
			if(i->factor==0)remove_next(i_prev);
			i=i_prev->next;
		}else{
			node * tmp=nodecpy(new_node);
			insert(i_prev,tmp);
			i=i_prev->next;
		}
		if(add_once)return;
		new_node=new_node->next;
	}
}
//多项式类
class Polynomial{
public:
	//储存一个链表，其节点的指数从大到小排列。
	node * items;
	//初始化链表头。
	void init(){
		items=new node;
		items->exp=EXP_HEAD_NODE;
		items->factor=1;
		items->next=NULL;
	}
	Polynomial(){
		items=NULL;
	}
	//深拷贝初始化
	Polynomial(Polynomial & src){
		items=nodescpy(src.items);
	}
	//用链表初始化
	Polynomial(node * new_items){
		if(new_items->exp!=EXP_HEAD_NODE){
			init();
			items->next=new_items;
		}else{
			items=new_items;
		}
	}
	//获取最高次项
	node * getLeadingItem(){
		return items->next;
	}
	//运算符重载
    Polynomial operator + (const Polynomial & B){
		Polynomial new_poly;
		new_poly.items=nodescpy(B.items);
		insert_and_sort(new_poly.items,items);
		return new_poly;
	}
	Polynomial operator - (const Polynomial & B){
		Polynomial new_poly;
		new_poly.items=nodescpyNegation(B.items);
		insert_and_sort(new_poly.items,items);
		return new_poly;
	}
	//运算符重载，乘上x的n次方。
	Polynomial operator ^(const int num){
		Polynomial new_poly;
		new_poly.items=nodescpy_exp_add(items,num);
		return new_poly;
	}
	//运算符重载，数乘和数除。
	Polynomial operator *(const double num){
		Polynomial new_poly;
		new_poly.items=nodescpy(items);
		nodesScalarMultiply(new_poly.items,num,0);
		return new_poly;
	}
	Polynomial operator /(const double num){
		Polynomial new_poly;
		new_poly.items=nodescpy(items);
		nodesScalarMultiply(new_poly.items,1/num,0);
		return new_poly;
	}
	//运算符重载，多项式乘。
	Polynomial operator * (const Polynomial & B){
		Polynomial new_poly;
		new_poly.init();
		node * i,*j;
		for(i=items;i;i=i->next){
			if(i->exp==EXP_HEAD_NODE)continue;
			for(j=B.items;j;j=j->next){
				if(j->exp==EXP_HEAD_NODE)continue;
				node * node_multiply=new node;
				node_multiply->exp=i->exp+j->exp;
				node_multiply->factor=i->factor*j->factor;
				node_multiply->next=NULL;
				insert_and_sort(new_poly.items,node_multiply);
			}
		}
		return new_poly;
	}
	//多项式除法
	void devide(Polynomial & B,Polynomial & result,Polynomial & remained_item){
		result.init();
		Polynomial tmp(*this);
		do{
			int exp_diff=tmp.getLeadingItem()->exp-B.getLeadingItem()->exp;
			double factor_scale=B.getLeadingItem()->factor/tmp.getLeadingItem()->factor;
			result=result+Polynomial(new_node(1/factor_scale,exp_diff));
			tmp=(tmp)-((B)^exp_diff)/factor_scale;
		}while(tmp.getLeadingItem() && tmp.getLeadingItem()->exp>=B.getLeadingItem()->exp);
		remained_item=tmp;

	}
	//运算符重载，多项式除法
	Polynomial operator / (Polynomial & B){
		Polynomial result;
		Polynomial remained_item;
		devide(B,result,remained_item);
		return result;
	}
	//运算符重载，多项式除法取余
	Polynomial operator % (Polynomial & B){
		Polynomial result;
		Polynomial remained_item;
		devide(B,result,remained_item);
		return remained_item;
	}
	//文件输出
	void output(ofstream & file){
		node *i;
		bool first_item=true;
		for(i=items;i;i=i->next){
			if(i->exp==EXP_HEAD_NODE)continue;
			if(i->factor<0)file<<"-";
			else if(!first_item)file<<"+";
			if(i->exp!=0){
				if(abs(i->factor)!=1){
					
					if(i->factor-(int)(i->factor)==0)file<<std::fixed<<setprecision(0)<<abs(i->factor);
				    else file<<std::fixed<<setprecision(2)<<abs(i->factor);
				}
				file<<"x"<<i->exp;
			}else{
				if(i->factor-(int)(i->factor)==0)file<<std::fixed<<setprecision(0)<<abs(i->factor);
				else file<<setprecision(3)<<abs(i->factor);
			}

			file<<" ";
			if(first_item)first_item=!first_item;
		}
		file<<"\n";
	}
	//文件输入
	void input(ifstream & file){
		char str[100];
		init();
		node * tmp=items;
		while(file.get()=='('){
			tmp->next=new node;
			file.getline(str,100,')');
			sscanf(str,"%lf,%d",&tmp->next->factor,&tmp->next->exp);
			if(tmp->next->exp==0 && tmp->next->factor==0){
				delete(tmp->next);	
				tmp->next=NULL;
			}else{
				tmp=tmp->next;
			}
		}
	}

};
//求微分分别输出。
void diff(ofstream & out,Polynomial & A,Polynomial & B){
	Polynomial A_diff(A);
	Polynomial B_diff(B);
	nodesDiff(A_diff.items);
	A_diff.output(out);
	nodesDiff(B_diff.items);
	B_diff.output(out);
}
int main(){
	Polynomial poly1,poly2,poly3,poly4;
	ifstream in("infile.txt");
	ofstream out("outfile.txt");
	poly1.input(in);
	poly2.input(in);
	char str[10];
	in.getline(str,10,'(');
	if(strcmp(str,"AND")==0){
		poly3=poly1+poly2;
	}
	if(strcmp(str,"SUB")==0){
		poly3=poly1-poly2;
	}
	if(strcmp(str,"MUL")==0){
		poly3=poly1*poly2;
	}
	if(strcmp(str,"DIV")==0){
		poly3=poly1/poly2;
		poly3.output(out);
		poly3=poly1%poly2;
		if(poly3.getLeadingItem())out<<"remainder:";		
	}
	if(strcmp(str,"DIFF")==0){
		diff(out,poly1,poly2);
	}else{
		poly3.output(out);
	}
		out.close();
}