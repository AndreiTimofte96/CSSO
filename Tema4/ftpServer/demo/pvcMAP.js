
// const t0 = new Date().getTime();

const paths = [
  {
    coord: { lat: 45.64092778836502, lng: 25.64208984375 },
    name: "Brasov"
  },
  {
    coord: { lat: 47.16730970131578, lng: 27.186669921875 },
    name: "Iasi",
  },
  {
    coord: { lat: 47.46730970131578, lng: 27.486669921875 },
    name: "Iasi1",
  },
  {
    coord: { lat: 47.56730970131578, lng: 27.386669921875 },
    name: "Iasi2",
  },
  {
    coord: { lat: 47.66730970131578, lng: 27.286669921875 },
    name: "Iasi3",
  },
  {
    coord: { lat: 47.76730970131578, lng: 27.186669921875 },
    name: "Iasi4",
  },
  {
    coord: { lat: 47.86730970131578, lng: 27.086669921875 },
    name: "Iasi5",
  },
  {
    coord: { lat: 46.78501604269254, lng: 23.5986328125 },
    name: "Cluj",
  },
  {
    coord: { lat: 46.532413816559455, lng: 24.7357177734375 },
    name: "Tg Mures",
  },
  {
    coord: { lat: 45.43508099838452, lng: 28.037109375 },
    name: "Galati",
  },
  {
    coord: { lat: 44.449467536006935, lng: 26.081542968749996 },
    name: "Bucuresti",
  },
  {
    coord: { lat: 45.805828539928356, lng: 21.24755859375 },
    name: "Timisoara",
  },
  {
    coord: { lat: 44.3002644115815, lng: 23.785400390625 },
    name: "Craiova",
  },
  {
    coord: { lat: 47.754097979680026, lng: 26.71875 },
    name: "Botosani",
  },
];

let pDis;
let traseu = traseuMin = [0];
let lungime = 0;
let lungimeMin = Infinity;
let vizitat = new Array(paths.length).fill(false);
vizitat[0] = true;


const computeTwoPointsDistance = (p1, p2) => {
  if ((p1.lat === p2.lat) && (p1.lng === p2.lng)) {
    return 0;
  }
  var radlat1 = Math.PI * p1.lat / 180;
  var radlat2 = Math.PI * p2.lat / 180;
  var theta = p1.lng - p2.lng;
  var radtheta = Math.PI * theta / 180;
  var dist = Math.sin(radlat1) * Math.sin(radlat2) + Math.cos(radlat1) * Math.cos(radlat2) * Math.cos(radtheta);
  if (dist > 1) {
    dist = 1;
  }
  dist = Math.acos(dist);
  dist = dist * 180 / Math.PI;
  dist = dist * 60 * 1.1515;
  dist = dist * 1.609344; // from miles to kms

  return dist;
}

const bkt = (nod) => {
  if (lungime >= lungimeMin) return;

  if (nod === pDis.length) {
    if (lungime < lungimeMin) {
      lungimeMin = lungime;
      traseuMin = [...traseu];
    }
    return;
  }

  for (let index = 1; index < pDis.length; index++) {
    const nodPrecedent = traseu[nod - 1];
    if (!vizitat[index] && pDis[nodPrecedent][index]) {

      vizitat[index] = true;
      traseu.push(index);
      lungime += pDis[nodPrecedent][index];

      bkt(nod + 1);
      traseu.pop();
      lungime -= pDis[nodPrecedent][index];
      vizitat[index] = false;

    }
  }
}

const computeDistancesBetweenAllCoordinates = (paths) => {
  const matrix = new Array(paths.length).fill(0)
    .map(() => new Array(paths.length).fill(0));

  for (index = 0; index < paths.length - 1; index++) {
    for (jindex = index + 1; jindex < paths.length; jindex++) {
      const dist = computeTwoPointsDistance(paths[index].coord, paths[jindex].coord);
      matrix[index][jindex] = matrix[jindex][index] = dist;
    }
  }
  return matrix;
}



const main = () => {
  pDis = computeDistancesBetweenAllCoordinates(paths);

  bkt(1);
  console.log(lungimeMin);
  for (let index = 0; index < traseuMin.length; index++) {
    console.log(paths[traseuMin[index]].name);
  }
}
main();

// const t1 = new Date().getTime();
// console.log("Call to doSomething took " + (t1 - t0) + " milliseconds.");
