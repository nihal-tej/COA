#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
using namespace std;
vector<int> mainmem;

class cachefile_direct_wb
{
    int offbits, idxbits, numidx, miss, hittime, misstime, nblocks, hit, words, word, j;

    int getindex(int u)
    {
        return (u >> offbits) & (numidx - 1);
    }

    int gettag(int u)
    {
        return u >> (offbits + idxbits);
    }

    int getraddress(int u)
    {
        return (u >> offbits) << offbits;
    }

    int getword(int u)
    {
        int byteoffsetbits = offbits - words;
        return (u >> byteoffsetbits) & ((1 << words) - 1);
    }

    vector<int> ramvalues;
    vector<int> va, tag;
    vector<vector<int>> data;

public:
    cachefile_direct_wb(int n, int o, int t, int i, int h, int m, int nb, int ws)
    {
        va.resize(n, 0);
        tag.resize(n, -1);
        j = 0;
        words = ws;
        word = 1 << ws;

        data = vector<vector<int>>(n, vector<int>(word, -1));
        ramvalues = mainmem;
        offbits = o;
        idxbits = t;
        numidx = i;
        hittime = h;
        misstime = m;
        nblocks = nb;

        hit = 0;
        miss = 0;
    }

    void access_write(int addr, int val)
    {
        int id = getindex(addr);
        int t = gettag(addr);

        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);

        int wi = getword(addr);

        if (!va[id] || (tag[id] != t && va[id]))
        {
            if (va[id] == 2)
            {
                int oldBaseByte = (tag[id] << (idxbits + offbits)) | (id << offbits);
                int oldBaseWord = oldBaseByte >> (offbits - words);

                for (int i = 0; i < word; i++)
                    ramvalues[oldBaseWord + i] = data[id][i];
                j++;
            }

            // bring new block (using raddr)
            va[id] = 1;
            tag[id] = t;

            for (int i = 0; i < word; i++)
                data[id][i] = ramvalues[basew + i];

            // write into cache
            data[id][wi] = val;
            va[id] = 2;

            miss++;
        }
        else
        {
            data[id][wi] = val;
            va[id] = 2;
            hit++;
        }
    }

    void access_read(int addr)
    {
        int id = getindex(addr);
        int t = gettag(addr);

        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);

        int wi = getword(addr);

        if (!va[id] || (tag[id] != t && va[id]))
        {
            if (va[id] == 2)
            {
                int oldBaseByte = (tag[id] << (idxbits + offbits)) | (id << offbits);
                int oldBaseWord = oldBaseByte >> (offbits - words);

                for (int i = 0; i < word; i++)
                    ramvalues[oldBaseWord + i] = data[id][i];
            }

            va[id] = 1;
            tag[id] = t;

            for (int i = 0; i < word; i++)
                data[id][i] = ramvalues[basew + i];

            miss++;
        }
        else
            hit++;
    }

    void printcache()
    {
        // for (int i = 0; i < (int)va.size(); i++)
        // {
        //     cout << "Line number:" << i << " " << va[i] << " " << tag[i];
        //     for (int j = 0; j < word; j++)
        //         cout << " " << data[i][j];
        //     cout << endl;
        // }
        double amat = (hittime + (misstime * miss / (hit + miss)));
        cout << "AMAT:" << amat << endl;

        int time = hittime * hit + misstime * miss;

        cout << "Bandwidth:" << (double)(hit + miss) * (word) / time << endl;
        cout << "Number of writes into the memory:" << j << endl;
    }
};
class cachefile_direct_wt
{
    int offbits, idxbits, numidx, miss, hittime, misstime, nblocks, hit, words, word, j;

    int getindex(int u)
    {
        return (u >> offbits) & (numidx - 1);
    }

    int gettag(int u)
    {
        return u >> (offbits + idxbits);
    }

    int getraddress(int u)
    {
        return (u >> offbits) << offbits;
    }

    int getword(int u)
    {
        int byteoffsetbits = offbits - words;
        return (u >> byteoffsetbits) & ((1 << words) - 1);
    }

    vector<int> va, tag;
    vector<vector<int>> data;
    vector<int> ramvalues;

public:
    cachefile_direct_wt(int n, int o, int t, int i, int h, int m, int nb, int ws)
    {
        va.resize(n, 0);
        tag.resize(n, -1);
        j = 0;
        words = ws;
        word = 1 << ws;
        ramvalues = mainmem;
        data = vector<vector<int>>(n, vector<int>(word, -1));
        offbits = o;
        idxbits = t;
        numidx = i;
        hittime = h;
        misstime = m;
        nblocks = nb;

        hit = 0;
        miss = 0;
    }

    void access_write(int addr, int val)
    {
        int id = getindex(addr);
        int t = gettag(addr);

        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);

        int wi = getword(addr);

        if (!va[id] || (tag[id] != t && va[id]))
        {

            va[id] = 1;
            tag[id] = t;

            for (int i = 0; i < word; i++)
                data[id][i] = ramvalues[basew + i];

            miss++;
        }
        else
        {
            hit++;
        }
        int oldBaseByte = (tag[id] << (idxbits + offbits)) | (id << offbits);
        int oldBaseWord = oldBaseByte >> (offbits - words);
        ramvalues[oldBaseWord + wi] = val;
        j++;
        data[id][wi] = val;
    }

    void access_read(int addr)
    {
        int id = getindex(addr);
        int t = gettag(addr);

        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);

        int wi = getword(addr);

        if (!va[id] || (tag[id] != t && va[id]))
        {
            va[id] = 1;
            tag[id] = t;

            for (int i = 0; i < word; i++)
                data[id][i] = ramvalues[basew + i];
            miss++;
        }
        else
            hit++;
    }

    void printcache()
    {
        // for (int i = 0; i < (int)va.size(); i++)
        // {
        //     cout << "Line number:" << i << " " << va[i] << " " << tag[i];
        //     for (int j = 0; j < word; j++)
        //         cout << " " << data[i][j];
        //     cout << endl;
        // }
        double amat = hittime + (misstime * miss / (hit + miss));
        cout << "AMAT:" << amat << endl;

        int time = hittime * hit + misstime * miss;
        cout << "Bandwidth:" << (double)(hit + miss) * (word) / time << endl;
        cout << "Number of writes into the memory:" << j << endl;
    }
};
class cachefile_associative_wb
{
    int offbits, numidx, miss, hittime, misstime, nblocks, hit, words, word, j, block;
    int gettag(int u)
    {
        return u >> (offbits);
    }

    int getraddress(int u)
    {
        return (u >> offbits) << offbits;
    }

    int getword(int u)
    {
        int byteoffsetbits = offbits - words;
        return (u >> byteoffsetbits) & ((1 << words) - 1);
    }

    vector<int> va, tag;
    vector<vector<int>> data;
    vector<int> ramvalues;

public:
    cachefile_associative_wb(int o, int h, int m, int nb, int ws, int l)
    {
        va.resize(l, 0);
        tag.resize(l, -1);

        words = ws;
        word = 1 << ws;
        block = l;
        data = vector<vector<int>>(l, vector<int>(word, -1));
        ramvalues = mainmem;
        offbits = o;
        hittime = h;
        misstime = m;
        nblocks = nb;
        j = 0;
        hit = 0;
        miss = 0;
    }

    void access_write(int addr, int val)
    {
        int t = gettag(addr);
        int id = -1;
        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);

        int wi = getword(addr);
        for (int i = 0; i < block; i++)
        {
            if ((va[i] && tag[i] == t))
            {
                data[i][wi] = val;
                va[i] = 2;
                hit++;
                return;
            }
            if (!va[i])
                id = i;
        }
        if (id == -1)
            id = rand() % block;
        if (va[id] == 2)
        {
            int oldBaseByte = (tag[id] << (offbits)) | (id << offbits);
            int oldBaseWord = oldBaseByte >> (offbits - words);

            for (int i = 0; i < word; i++)
                ramvalues[oldBaseWord + i] = data[id][i];
            j++;
        }
        va[id] = 1;
        tag[id] = t;
        for (int i = 0; i < word; i++)
            data[id][i] = ramvalues[basew + i];
    }

    void access_read(int addr)
    {
        int id = -1;
        int t = gettag(addr);
        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);
        int wi = getword(addr);
        for (int i = 0; i < block; i++)
        {
            if ((va[i] && tag[i] == t))
            {
                hit++;
                return;
            }
            if (!va[i])
                id = i;
        }
        if (id == -1)
            id = rand() % block;
        if (va[id] == 2)
        {
            int oldBaseByte = (tag[id] << (offbits)) | (id << offbits);
            int oldBaseWord = oldBaseByte >> (offbits - words);

            for (int i = 0; i < word; i++)
                ramvalues[oldBaseWord + i] = data[id][i];
            j++;
        }

        va[id] = 1;
        tag[id] = t;
        for (int i = 0; i < word; i++)
            data[id][i] = ramvalues[basew + i];
    }

    void printcache()
    {
        // for (int i = 0; i < (int)va.size(); i++)
        // {
        //     cout << "Line number:" << i << " " << va[i] << " " << tag[i];
        //     for (int j = 0; j < word; j++)
        //         cout << " " << data[i][j];
        //     cout << endl;
        // }
        double amat = hittime + (misstime * miss / (hit + miss));
        cout << "AMAT:" << amat << endl;

        int time = hittime * hit + misstime * miss;

        cout << "Bandwidth:" << (double)(hit + miss) * (word) / time << endl;
        cout << "Number of writes into the memory:" << j << endl;
    }
};
class cachefile_associative_wt
{
    int offbits, numidx, miss, hittime, misstime, nblocks, hit, words, word, j, block;
    int gettag(int u)
    {
        return u >> (offbits);
    }

    int getraddress(int u)
    {
        return (u >> offbits) << offbits;
    }

    int getword(int u)
    {
        int byteoffsetbits = offbits - words;
        return (u >> byteoffsetbits) & ((1 << words) - 1);
    }

    vector<int> va, tag;
    vector<vector<int>> data;
    vector<int> ramvalues;

public:
    cachefile_associative_wt(int o, int h, int m, int nb, int ws, int l)
    {
        va.resize(l, 0);
        tag.resize(l, -1);

        words = ws;
        word = 1 << ws;
        block = l;
        data = vector<vector<int>>(l, vector<int>(word, -1));
        ramvalues = mainmem;
        offbits = o;
        hittime = h;
        misstime = m;
        nblocks = nb;
        j = 0;
        hit = 0;
        miss = 0;
    }

    void access_write(int addr, int val)
    {
        int t = gettag(addr);
        int id = -1;
        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);

        int wi = getword(addr);
        for (int i = 0; i < block; i++)
        {
            if ((va[i] && tag[i] == t))
            {
                int oldBaseByte = (tag[id] << (offbits)) | (id << offbits);
                int oldBaseWord = oldBaseByte >> (offbits - words);
                data[i][wi] = val;
                ramvalues[oldBaseWord + wi] = data[i][wi];
                va[i] = 2;
                hit++;
                j++;
                return;
            }
            if (!va[i])
                id = i;
        }
        if (id == -1)
            id = rand() % block;
        if (va[id] == 2)
        {
            int oldBaseByte = (tag[id] << (offbits)) | (id << offbits);
            int oldBaseWord = oldBaseByte >> (offbits - words);

            for (int i = 0; i < word; i++)
                ramvalues[oldBaseWord + i] = data[id][i];
            j++;
        }
        va[id] = 1;
        tag[id] = t;
        for (int i = 0; i < word; i++)
            data[id][i] = ramvalues[basew + i];
    }

    void access_read(int addr)
    {
        int id = -1;
        int t = gettag(addr);
        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);
        int wi = getword(addr);
        for (int i = 0; i < block; i++)
        {
            if ((va[i] && tag[i] == t))
            {
                hit++;
                return;
            }
            if (!va[i])
                id = i;
        }
        if (id == -1)
            id = rand() % block;
        if (va[id] == 2)
        {
            int oldBaseByte = (tag[id] << (offbits)) | (id << offbits);
            int oldBaseWord = oldBaseByte >> (offbits - words);

            for (int i = 0; i < word; i++)
                ramvalues[oldBaseWord + i] = data[id][i];
            j++;
        }

        va[id] = 1;
        tag[id] = t;
        for (int i = 0; i < word; i++)
            data[id][i] = ramvalues[basew + i];
    }

    void printcache()
    {
        // for (int i = 0; i < (int)va.size(); i++)
        // {
        //     cout << "Line number:" << i << " " << va[i] << " " << tag[i];
        //     for (int j = 0; j < word; j++)
        //         cout << " " << data[i][j];
        //     cout << endl;
        // }
        double amat = hittime + (misstime * miss / (hit + miss));
        cout << "AMAT:" << amat << endl;

        int time = hittime * hit + misstime * miss;

        cout << "Bandwidth:" << (double)(hit + miss) * (word) / time << endl;
        cout << "Number of writes into the memory:" << j << endl;
    }
};
class cachefile_set_associative_wt
{
    int offbits, n, idxbits, numidx, miss, hittime, misstime, nblocks, hit, words, word, ways, j;

    int gettag(int u)
    {
        return u >> (offbits + idxbits);
    }

    int getraddress(int u)
    {
        return (u >> offbits) << offbits;
    }

    int getway(int u)
    {
        return (u >> offbits) % ways;
    }

    int getword(int u)
    {
        int byteoffsetbits = offbits - words;
        return (u >> byteoffsetbits) & ((1 << words) - 1);
    }

    vector<vector<int>> va, tag;
    vector<vector<vector<int>>> data;
    vector<int> ramvalues;

public:
    cachefile_set_associative_wt(int n, int o, int t, int i, int h, int m, int nb, int ws, int w)
    {
        ways = w;
        this->n = n;
        j = 0;

        va.resize(ways, vector<int>(n, 0));
        tag.resize(ways, vector<int>(n, 0));
        ramvalues = mainmem;
        words = ws;
        word = 1 << ws;

        data = vector<vector<vector<int>>>(ways, vector<vector<int>>(n, vector<int>(word, -1)));

        offbits = o;
        idxbits = t;
        numidx = i;

        hittime = h;
        misstime = m;
        nblocks = nb;

        hit = 0;
        miss = 0;
    }

    void writeback(int w, int id)
    {

        int oldBaseByte =
            (tag[w][id] << (offbits + idxbits)) |
            (id << offbits);

        int oldBaseWord = oldBaseByte >> (offbits - words);

        for (int i = 0; i < word; i++)
            ramvalues[oldBaseWord + i] = data[w][id][i];

        j++;
    }

    void access_write(int addr, int val)
    {
        int id = -1;
        int t = gettag(addr);
        int w = getway(addr);
        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);
        int wi = getword(addr);

        for (int i = 0; i < n; i++)
        {
            if (va[w][i] && tag[w][i] == t)
            {
                data[w][i][wi] = val;
                va[w][i] = 2;
                hit++;
                writeback(w, id);
                return;
            }
            if (!va[w][i])
                id = i;
        }

        miss++;

        if (id == -1)
            id = rand() % n;
        for (int i = 0; i < word; i++)
            data[w][id][i] = ramvalues[basew + i];

        tag[w][id] = t;
        data[w][id][wi] = val;
        writeback(w, id);
    }

    void access_read(int addr)
    {
        int id = -1;
        int t = gettag(addr);
        int w = getway(addr);
        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);
        int wi = getword(addr);

        for (int i = 0; i < n; i++)
        {
            if (va[w][i] && tag[w][i] == t)
            {
                hit++;
                return;
            }
            if (!va[w][i])
                id = i;
        }

        miss++;
        if (id == -1)
            id = rand() % n;
        for (int i = 0; i < word; i++)
            data[w][id][i] = ramvalues[basew + i];

        tag[w][id] = t;
        va[w][id] = 1;
    }

    void printcache()
    {
        // for (int i = 0; i < ways; i++)
        // {
        //     cout << "Way:" << i << endl;
        //     for (int j = 0; j < n; j++)
        //     {
        //         cout << "   Line " << j << " VA=" << va[i][j]
        //              << " TAG=" << tag[i][j] << " DATA:";
        //         for (int k = 0; k < word; k++)
        //             cout << " " << data[i][j][k];
        //         cout << endl;
        //     }
        // }

        double amat = hittime + (double)misstime * miss / (hit + miss);
        cout << "AMAT:" << amat << endl;

        int time = hittime * hit + misstime * miss;
        double bandwidth = (double)(hit + miss) * word / time;

        cout << "Bandwidth:" << bandwidth << endl;
        cout << "Number of writes into the memory:" << j << endl;
    }
};
class cachefile_set_associative_wb
{
    int offbits, n, idxbits, numidx, miss, hittime, misstime, nblocks, hit, words, word, ways, j;

    int gettag(int u)
    {
        return u >> (offbits + idxbits);
    }

    int getraddress(int u)
    {
        return (u >> offbits) << offbits;
    }

    int getway(int u)
    {
        return (u >> offbits) % ways;
    }

    int getword(int u)
    {
        int byteoffsetbits = offbits - words;
        return (u >> byteoffsetbits) & ((1 << words) - 1);
    }

    vector<vector<int>> va, tag;
    vector<vector<vector<int>>> data;
    vector<int> ramvalues;

public:
    cachefile_set_associative_wb(int n, int o, int t, int i, int h, int m, int nb, int ws, int w)
    {
        ways = w;
        this->n = n;
        j = 0;

        va.resize(ways, vector<int>(n, 0));
        tag.resize(ways, vector<int>(n, 0));
        ramvalues = mainmem;
        words = ws;
        word = 1 << ws;

        data = vector<vector<vector<int>>>(ways, vector<vector<int>>(n, vector<int>(word, -1)));

        offbits = o;
        idxbits = t;
        numidx = i;

        hittime = h;
        misstime = m;
        nblocks = nb;

        hit = 0;
        miss = 0;
    }

    void writeback(int w, int id)
    {
        if (va[w][id] == 2)
        {
            int oldBaseByte =
                (tag[w][id] << (offbits + idxbits)) |
                (id << offbits);

            int oldBaseWord = oldBaseByte >> (offbits - words);

            for (int i = 0; i < word; i++)
                ramvalues[oldBaseWord + i] = data[w][id][i];

            j++;
        }
    }

    void access_write(int addr, int val)
    {
        int id = -1;
        int t = gettag(addr);
        int w = getway(addr);
        int index = (addr >> offbits) & (numidx - 1);
        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);
        int wi = getword(addr);

        for (int i = 0; i < n; i++)
        {
            if (va[w][i] && tag[w][i] == t)
            {
                data[w][i][wi] = val;
                va[w][i] = 2;
                hit++;
                return;
            }
            if (!va[w][i])
                id = i;
        }

        miss++;

        if (id == -1)
            id = rand() % n;

        writeback(w, id);

        for (int i = 0; i < word; i++)
            data[w][id][i] = ramvalues[basew + i];

        tag[w][id] = t;
        data[w][id][wi] = val;
        va[w][id] = 2;
    }

    void access_read(int addr)
    {
        int id = -1;
        int t = gettag(addr);
        int w = getway(addr);
        int index = (addr >> offbits) & (numidx - 1);
        int raddr = getraddress(addr);
        int basew = raddr >> (offbits - words);
        int wi = getword(addr);

        for (int i = 0; i < n; i++)
        {
            if (va[w][i] && tag[w][i] == t)
            {
                hit++;
                return;
            }
            if (!va[w][i])
                id = i;
        }

        miss++;

        if (id == -1)
            id = rand() % n;
        writeback(w, id);
        for (int i = 0; i < word; i++)
            data[w][id][i] = ramvalues[basew + i];

        tag[w][id] = t;
        va[w][id] = 1;
    }

    void printcache()
    {
        // for (int i = 0; i < ways; i++)
        // {
        //     cout << "Way:" << i << endl;
        //     for (int j = 0; j < n; j++)
        //     {
        //         cout << "   Line " << j << " VA=" << va[i][j]
        //              << " TAG=" << tag[i][j] << " DATA:";
        //         for (int k = 0; k < word; k++)
        //             cout << " " << data[i][j][k];
        //         cout << endl;
        //     }
        // }

        double amat = hittime + (double)misstime * miss / (hit + miss);
        cout << "AMAT:" << amat << endl;

        int time = hittime * hit + misstime * miss;
        double bandwidth = (double)(hit + miss) * word / time;

        cout << "Bandwidth:" << bandwidth << endl;
        cout << "Number of writes into the memory:" << j << endl;
    }
};
int main()
{
    ifstream infile("input_cache.txt");
    int ram, cache, word, bsize, hittime, misstime, ways;

    infile >> ram >> cache >> bsize >> word >> ways >> hittime >> misstime;
    
    srand(time(0));

    int nram = ram / word;
    mainmem.resize(nram);

    for (int i = 0; i < nram; i++)
        mainmem[i] = rand() % 100;

    int ncache = cache / word;
    int nblocks = ram / bsize;
    int idx = cache / bsize;

    int offbits = log2(bsize);
    int idxbits = log2(idx);
    int ws = log2(bsize / word);
    
    cachefile_direct_wb c(idx, offbits, idxbits, idx, hittime, misstime, nblocks, ws);
    cachefile_direct_wt d(idx, offbits, idxbits, idx, hittime, misstime, nblocks, ws);
    cachefile_associative_wb e(offbits, hittime, misstime, nblocks, ws, cache / bsize);
    cachefile_associative_wt f(offbits, hittime, misstime, nblocks, ws, cache / bsize);
    cachefile_set_associative_wt g(idx, offbits, idxbits, idx, hittime, misstime, nblocks, ws, ways);
    cachefile_set_associative_wb h(idx, offbits, idxbits, idx, hittime, misstime, nblocks, ws, ways);

    ifstream infile1("input.txt");
    string line1;

    while (getline(infile1, line1))
    {
        if (line1.empty())
            continue;

        stringstream ss(line1);
        char type;
        int a, b;

        ss >> type >> a >> b;

        if (type == 'w')
        {
            c.access_write(a, b);
            d.access_write(a, b);
            e.access_write(a, b);
            f.access_write(a, b);
            g.access_write(a, b);
            h.access_write(a, b);
        }
        else if (type == 'r')
        {
            c.access_read(a);
            d.access_read(a);
            e.access_read(a);
            f.access_read(a);
            g.access_read(a);
            h.access_read(a);
        }
    }
    cout << "1.Direct-Mapped Cache with write back" << endl;
    c.printcache();
    cout << "2.Direct-Mapped Cache with write through" << endl;
    d.printcache();
    cout << "3.Associative Cache with write back" << endl;
    e.printcache();
    cout << "4.Associative Cache with write through" << endl;
    f.printcache();
    cout << "5.Set Associative Cache with write back" << endl;
    h.printcache();
    cout << "6.Set Associative Cache with write through" << endl;
    g.printcache();
    infile1.close();
    infile.close();
    return 0;
}